#include <iostream>
#include <iomanip>  // Для std::fixed и std::setprecision
#include <sstream>  // Для форматирования в строку
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <set>

// Отключаем предупреждения в WebSocket++
#pragma warning(push, 0)
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#pragma warning(pop)

#include <nlohmann/json.hpp>
#include "harvester.h"
#include <windows.h>

using json = nlohmann::json;
typedef websocketpp::server<websocketpp::config::asio> WebSocketServer;

static harvester harvester_Obj;
std::mutex g_modelMutex;
std::atomic<bool> g_running{true};

// Глобальный объект WebSocket сервера
WebSocketServer wsServer;

// Множество подключенных клиентов
std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> g_connections;
std::mutex g_connectionsMutex;

// Функция для округления числа до сотых
double roundToHundredths(double value) {
    return std::round(value * 100.0) / 100.0;
}

void rt_OneStep(void)
{
  static boolean_T OverrunFlag{ false };
  if (OverrunFlag) {
    return;
  }
  OverrunFlag = true;
  harvester_Obj.step();
  OverrunFlag = false;
}

// Функция для отправки данных ВСЕМ подключенным клиентам
void broadcastState()
{
  json response;
  response["totalDrop"] = roundToHundredths(harvester_Obj.harvester_Y.totalDrop);
  response["engineLoad"] = roundToHundredths(harvester_Obj.harvester_Y.engineLoad);
  response["status"] = "ok";

  std::lock_guard<std::mutex> lock(g_connectionsMutex);
  for (auto& hdl : g_connections) {
    try {
      wsServer.send(hdl, response.dump(), websocketpp::frame::opcode::text);
    } catch (...) {
      // Игнорируем ошибки отправки (клиент мог отключиться)
    }
  }
}

void startWebSocketServer()
{
  try {
    // Отключаем подробное логирование WebSocket++
    wsServer.set_access_channels(websocketpp::log::alevel::none);
    wsServer.clear_access_channels(websocketpp::log::alevel::frame_payload);

    // Инициализируем ASIO
    wsServer.init_asio();

    // Включаем переиспользование адреса
    wsServer.set_reuse_addr(true);

    // Обработчик входящих сообщений
    wsServer.set_message_handler(
      [](websocketpp::connection_hdl hdl, WebSocketServer::message_ptr msg) {
        try {
          auto data = json::parse(msg->get_payload());
          std::lock_guard<std::mutex> lock(g_modelMutex);

          // Обновляем параметры модели из JSON
          if (data.contains("nEngine"))
            harvester_Obj.harvester_U.nEngine = data["nEngine"];
          if (data.contains("sEngine"))
            harvester_Obj.harvester_U.sEngine = data["sEngine"];
          if (data.contains("sDrum"))
            harvester_Obj.harvester_U.sDrum = data["sDrum"];
          if (data.contains("nDrum"))
            harvester_Obj.harvester_U.nDrum = data["nDrum"];
          if (data.contains("sHeader"))
            harvester_Obj.harvester_U.sHeader = data["sHeader"];
          if (data.contains("nHeader"))
            harvester_Obj.harvester_U.nHeader = data["nHeader"];
          if (data.contains("sGrainConveyor"))
            harvester_Obj.harvester_U.sGrainConveyor = data["sGrainConveyor"];

        } catch (const std::exception& e) {
          json errorResponse;
          errorResponse["error"] = "Invalid JSON";
          errorResponse["details"] = e.what();
          wsServer.send(hdl, errorResponse.dump(), websocketpp::frame::opcode::text);
        }
      }
    );

    // Обработчик подключения клиента
    wsServer.set_open_handler(
      [](websocketpp::connection_hdl hdl) {
        std::cout << "Client connected!" << std::endl;

        // Добавляем клиента в список
        {
          std::lock_guard<std::mutex> lock(g_connectionsMutex);
          g_connections.insert(hdl);
        }

        // Отправляем текущее состояние новому клиенту
        json initResponse;
        initResponse["totalDrop"] = roundToHundredths(harvester_Obj.harvester_Y.totalDrop);
        initResponse["engineLoad"] = roundToHundredths(harvester_Obj.harvester_Y.engineLoad);
        initResponse["status"] = "connected";

        wsServer.send(hdl, initResponse.dump(), websocketpp::frame::opcode::text);
      }
    );

    // Обработчик отключения клиента
    wsServer.set_close_handler(
      [](websocketpp::connection_hdl hdl) {
        std::cout << "Client disconnected!" << std::endl;

        // Удаляем клиента из списка
        std::lock_guard<std::mutex> lock(g_connectionsMutex);
        g_connections.erase(hdl);
      }
    );

    // Запускаем сервер на порту 9001
    wsServer.listen(9001);
    wsServer.start_accept();

    std::cout << "WebSocket server started successfully on ws://127.0.0.1:9001" << std::endl;
    std::cout << "Waiting for client connections..." << std::endl;

    // Запускаем event loop ASIO (блокирующий вызов)
    wsServer.run();

  } catch (const websocketpp::exception& e) {
    std::cerr << "WebSocket server error: " << e.what() << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << std::endl;
  }
}

void stopWebSocketServer() {
  try {
    wsServer.stop_listening();
    wsServer.stop();
    std::cout << "WebSocket server stopped." << std::endl;
  } catch (...) {
    std::cerr << "Error stopping server" << std::endl;
  }
}

int_T main(int_T argc, const char *argv[])
{
  // Инициализация модели
  harvester_Obj.initialize();
  std::cout << "Model initialized successfully." << std::endl;

  // Начальные параметры
  harvester_Obj.harvester_U.sEngine = false;
  harvester_Obj.harvester_U.nEngine = 2500.0;
  harvester_Obj.harvester_U.sDrum = false;
  harvester_Obj.harvester_U.nDrum = 100.0;
  harvester_Obj.harvester_U.sHeader = false;
  harvester_Obj.harvester_U.nHeader = 10.0;
  harvester_Obj.harvester_U.sGrainConveyor = false;

  // Запускаем WebSocket сервер в отдельном потоке
  std::thread wsThread(startWebSocketServer);
  wsThread.detach();

  // Даём серверу время на запуск
  Sleep(1000);

  // Основной цикл симуляции
  int step = 0;
  while(g_running) {
    {
      std::lock_guard<std::mutex> lock(g_modelMutex);
      rt_OneStep();
      // Отправляем данные ВСЕМ подключенным клиентам на каждом шаге
      broadcastState();
    }

    // Вывод состояния в консоль с округлением до сотых
    if (step % 10 == 0) {
      std::cout << std::fixed << std::setprecision(2)
                << "Step " << step
                << " | Total Drop: " << harvester_Obj.harvester_Y.totalDrop
                << " | Engine RPM: " << harvester_Obj.harvester_Y.engineLoad
                << std::endl;
    }

    step++;
    Sleep(100); // 100 мс между шагами = 10 Гц
  }

  // Корректное завершение
  std::cout << "Shutting down..." << std::endl;
  stopWebSocketServer();
  Sleep(500); // Даём время на завершение

  return 0;
}