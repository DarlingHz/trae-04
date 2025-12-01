#include <httplib.h>
#include <nlohmann/json.hpp>
#include "service/auth_service.h"
#include "service/card_service.h"
#include "service/tag_service.h"
#include "auth/simple_jwt.h"
#include "util/json.h"

using json = nlohmann::json;
using namespace httplib;

int main() {
  // 数据库路径
  std::string db_path = "data/db.sqlite3";
  std::string jwt_secret = "secret_key";
  
  // 初始化服务
  auto auth_service = std::make_shared<AuthService>(db_path, jwt_secret);
  auto card_service = std::make_shared<CardService>(db_path);
  auto tag_service = std::make_shared<TagService>(db_path);
  
  // 初始化JWT工具
  SimpleJWT jwt(jwt_secret);
  
  Server svr;
  
  // 用户注册接口
  svr.Post("/api/register", [&](const Request& req, Response& res) {
    try {
      auto body = json::parse(req.body);
      std::string email = body["email"];
      std::string password = body["password"];
      
      auto result = auth_service->Register(email, password);
      if (result.IsSuccess()) {
        json response = {
          {"code", 0},
          {"message", "注册成功"},
          {"data", result.GetData()}
        };
        res.set_content(response.dump(), "application/json");
      } else {
        json response = {
          {"code", 1},
          {"message", result.GetError()},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      }
    } catch (const std::exception& e) {
      json response = {
        {"code", 500},
        {"message", e.what()},
        {"data", nullptr}
      };
      res.set_content(response.dump(), "application/json");
    }
  });
  
  // 用户登录接口
  svr.Post("/api/login", [&](const Request& req, Response& res) {
    try {
      auto body = json::parse(req.body);
      std::string email = body["email"];
      std::string password = body["password"];
      
      auto result = auth_service->Login(email, password);
      if (result.IsSuccess()) {
        json response = {
          {"code", 0},
          {"message", "登录成功"},
          {"data", result.GetData()}
        };
        res.set_content(response.dump(), "application/json");
      } else {
        json response = {
          {"code", 1},
          {"message", result.GetError()},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      }
    } catch (const std::exception& e) {
      json response = {
        {"code", 500},
        {"message", e.what()},
        {"data", nullptr}
      };
      res.set_content(response.dump(), "application/json");
    }
  });
  
  // 创建卡片接口
  svr.Post("/api/cards", [&](const Request& req, Response& res) {
    try {
      // 从Authorization头中获取token
      auto auth_header = req.get_header_value("Authorization");
      if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        json response = {
          {"code", 401},
          {"message", "未授权访问"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      std::string token = auth_header.substr(7);
      bool is_valid = jwt.VerifyToken(token);
      if (!is_valid) {
        json response = {
          {"code", 401},
          {"message", "token无效"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      auto claims = jwt.GetClaims(token);
      int user_id = std::stoi(claims["user_id"]);
      
      auto body = json::parse(req.body);
      std::string title = body["title"];
      std::string content = body["content"];
      std::vector<std::string> tags = body.value("tags", std::vector<std::string>());
      
      auto result = card_service->CreateCard(user_id, title, content, tags);
      if (result.IsSuccess()) {
        json response = {
          {"code", 0},
          {"message", "创建成功"},
          {"data", {"card_id", result.GetData()}}
        };
        res.set_content(response.dump(), "application/json");
      } else {
        json response = {
          {"code", 1},
          {"message", result.GetError()},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      }
    } catch (const std::exception& e) {
      json response = {
        {"code", 500},
        {"message", e.what()},
        {"data", nullptr}
      };
      res.set_content(response.dump(), "application/json");
    }
  });
  
  // 获取卡片列表接口
  svr.Get("/api/cards", [&](const Request& req, Response& res) {
    try {
      // 从Authorization头中获取token
      auto auth_header = req.get_header_value("Authorization");
      if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        json response = {
          {"code", 401},
          {"message", "未授权访问"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      std::string token = auth_header.substr(7);
      bool is_valid = jwt.VerifyToken(token);
      if (!is_valid) {
        json response = {
          {"code", 401},
          {"message", "token无效"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      auto claims = jwt.GetClaims(token);
      int user_id = std::stoi(claims["user_id"]);
      
      // 获取查询参数
      int page = req.has_param("page") ? std::stoi(req.get_param_value("page")) : 1;
      int page_size = req.has_param("page_size") ? std::stoi(req.get_param_value("page_size")) : 10;
      std::string sort = req.has_param("sort") ? req.get_param_value("sort") : "updated_at";
      std::string tags_str = req.has_param("tags") ? req.get_param_value("tags") : "";
      std::string search = req.has_param("search") ? req.get_param_value("search") : "";
      
      // 解析标签列表
      std::vector<std::string> tags;
      if (!tags_str.empty()) {
        std::stringstream ss(tags_str);
        std::string tag;
        while (std::getline(ss, tag, ',')) {
          tags.push_back(tag);
        }
      }
      
      auto result = card_service->GetCardList(user_id, page, page_size, sort, tags, search);
      if (result.IsSuccess()) {
        json response = {
          {"code", 0},
          {"message", "获取成功"},
          {"data", result.GetData()}
        };
        res.set_content(response.dump(), "application/json");
      } else {
        json response = {
          {"code", 1},
          {"message", result.GetError()},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      }
    } catch (const std::exception& e) {
      json response = {
        {"code", 500},
        {"message", e.what()},
        {"data", nullptr}
      };
      res.set_content(response.dump(), "application/json");
    }
  });
  
  // 获取卡片详情接口
  svr.Get(R"(/api/cards/(\d+))", [&](const Request& req, Response& res) {
    try {
      // 从Authorization头中获取token
      auto auth_header = req.get_header_value("Authorization");
      if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        json response = {
          {"code", 401},
          {"message", "未授权访问"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      std::string token = auth_header.substr(7);
      bool is_valid = jwt.VerifyToken(token);
      if (!is_valid) {
        json response = {
          {"code", 401},
          {"message", "token无效"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      auto claims = jwt.GetClaims(token);
      int user_id = std::stoi(claims["user_id"]);
      int card_id = std::stoi(req.matches[1]);
      
      auto result = card_service->GetCardById(user_id, card_id);
      if (result.IsSuccess()) {
        json response = {
          {"code", 0},
          {"message", "获取成功"},
          {"data", result.GetData()}
        };
        res.set_content(response.dump(), "application/json");
      } else {
        json response = {
          {"code", 1},
          {"message", result.GetError()},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      }
    } catch (const std::exception& e) {
      json response = {
        {"code", 500},
        {"message", e.what()},
        {"data", nullptr}
      };
      res.set_content(response.dump(), "application/json");
    }
  });
  
  // 更新卡片接口
  svr.Put(R"(/api/cards/(\d+))", [&](const Request& req, Response& res) {
    try {
      // 从Authorization头中获取token
      auto auth_header = req.get_header_value("Authorization");
      if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        json response = {
          {"code", 401},
          {"message", "未授权访问"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      std::string token = auth_header.substr(7);
      bool is_valid = jwt.VerifyToken(token);
      if (!is_valid) {
        json response = {
          {"code", 401},
          {"message", "token无效"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      auto claims = jwt.GetClaims(token);
      int user_id = std::stoi(claims["user_id"]);
      int card_id = std::stoi(req.matches[1]);
      
      auto body = json::parse(req.body);
      std::string title = body["title"];
      std::string content = body["content"];
      std::vector<std::string> tags = body.value("tags", std::vector<std::string>());
      
      auto result = card_service->UpdateCard(user_id, card_id, title, content, tags);
      if (result.IsSuccess()) {
        json response = {
          {"code", 0},
          {"message", "更新成功"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      } else {
        json response = {
          {"code", 1},
          {"message", result.GetError()},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      }
    } catch (const std::exception& e) {
      json response = {
        {"code", 500},
        {"message", e.what()},
        {"data", nullptr}
      };
      res.set_content(response.dump(), "application/json");
    }
  });
  
  // 删除卡片接口
  svr.Delete(R"(/api/cards/(\d+))", [&](const Request& req, Response& res) {
    try {
      // 从Authorization头中获取token
      auto auth_header = req.get_header_value("Authorization");
      if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        json response = {
          {"code", 401},
          {"message", "未授权访问"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      std::string token = auth_header.substr(7);
      bool is_valid = jwt.VerifyToken(token);
      if (!is_valid) {
        json response = {
          {"code", 401},
          {"message", "token无效"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      auto claims = jwt.GetClaims(token);
      int user_id = std::stoi(claims["user_id"]);
      int card_id = std::stoi(req.matches[1]);
      
      auto result = card_service->DeleteCard(user_id, card_id);
      if (result.IsSuccess()) {
        json response = {
          {"code", 0},
          {"message", "删除成功"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      } else {
        json response = {
          {"code", 1},
          {"message", result.GetError()},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      }
    } catch (const std::exception& e) {
      json response = {
        {"code", 500},
        {"message", e.what()},
        {"data", nullptr}
      };
      res.set_content(response.dump(), "application/json");
    }
  });
  
  // 获取标签列表接口
  svr.Get("/api/tags", [&](const Request& req, Response& res) {
    try {
      // 从Authorization头中获取token
      auto auth_header = req.get_header_value("Authorization");
      if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        json response = {
          {"code", 401},
          {"message", "未授权访问"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      std::string token = auth_header.substr(7);
      bool is_valid = jwt.VerifyToken(token);
      if (!is_valid) {
        json response = {
          {"code", 401},
          {"message", "token无效"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      auto claims = jwt.GetClaims(token);
      int user_id = std::stoi(claims["user_id"]);
      
      auto result = tag_service->GetTagList(user_id);
      if (result.IsSuccess()) {
        json response = {
          {"code", 0},
          {"message", "获取成功"},
          {"data", result.GetData()}
        };
        res.set_content(response.dump(), "application/json");
      } else {
        json response = {
          {"code", 1},
          {"message", result.GetError()},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      }
    } catch (const std::exception& e) {
      json response = {
        {"code", 500},
        {"message", e.what()},
        {"data", nullptr}
      };
      res.set_content(response.dump(), "application/json");
    }
  });
  
  // 创建标签接口
  svr.Post("/api/tags", [&](const Request& req, Response& res) {
    try {
      // 从Authorization头中获取token
      auto auth_header = req.get_header_value("Authorization");
      if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        json response = {
          {"code", 401},
          {"message", "未授权访问"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      std::string token = auth_header.substr(7);
      bool is_valid = jwt.VerifyToken(token);
      if (!is_valid) {
        json response = {
          {"code", 401},
          {"message", "token无效"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      auto claims = jwt.GetClaims(token);
      int user_id = std::stoi(claims["user_id"]);
      
      auto body = json::parse(req.body);
      std::string name = body["name"];
      
      auto result = tag_service->CreateTag(user_id, name);
      if (result.IsSuccess()) {
        json response = {
          {"code", 0},
          {"message", "创建成功"},
          {"data", {"tag_id", result.GetData()}}
        };
        res.set_content(response.dump(), "application/json");
      } else {
        json response = {
          {"code", 1},
          {"message", result.GetError()},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      }
    } catch (const std::exception& e) {
      json response = {
        {"code", 500},
        {"message", e.what()},
        {"data", nullptr}
      };
      res.set_content(response.dump(), "application/json");
    }
  });
  
  // 更新标签接口
  svr.Put(R"(/api/tags/(\d+))", [&](const Request& req, Response& res) {
    try {
      // 从Authorization头中获取token
      auto auth_header = req.get_header_value("Authorization");
      if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        json response = {
          {"code", 401},
          {"message", "未授权访问"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      std::string token = auth_header.substr(7);
      bool is_valid = jwt.VerifyToken(token);
      if (!is_valid) {
        json response = {
          {"code", 401},
          {"message", "token无效"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      auto claims = jwt.GetClaims(token);
      int user_id = std::stoi(claims["user_id"]);
      int tag_id = std::stoi(req.matches[1]);
      
      auto body = json::parse(req.body);
      std::string name = body["name"];
      
      auto result = tag_service->UpdateTag(user_id, tag_id, name);
      if (result.IsSuccess()) {
        json response = {
          {"code", 0},
          {"message", "更新成功"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      } else {
        json response = {
          {"code", 1},
          {"message", result.GetError()},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      }
    } catch (const std::exception& e) {
      json response = {
        {"code", 500},
        {"message", e.what()},
        {"data", nullptr}
      };
      res.set_content(response.dump(), "application/json");
    }
  });
  
  // 删除标签接口
  svr.Delete(R"(/api/tags/(\d+))", [&](const Request& req, Response& res) {
    try {
      // 从Authorization头中获取token
      auto auth_header = req.get_header_value("Authorization");
      if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        json response = {
          {"code", 401},
          {"message", "未授权访问"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      std::string token = auth_header.substr(7);
      bool is_valid = jwt.VerifyToken(token);
      if (!is_valid) {
        json response = {
          {"code", 401},
          {"message", "token无效"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
        return;
      }
      
      auto claims = jwt.GetClaims(token);
      int user_id = std::stoi(claims["user_id"]);
      int tag_id = std::stoi(req.matches[1]);
      
      auto result = tag_service->DeleteTag(user_id, tag_id);
      if (result.IsSuccess()) {
        json response = {
          {"code", 0},
          {"message", "删除成功"},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      } else {
        json response = {
          {"code", 1},
          {"message", result.GetError()},
          {"data", nullptr}
        };
        res.set_content(response.dump(), "application/json");
      }
    } catch (const std::exception& e) {
      json response = {
        {"code", 500},
        {"message", e.what()},
        {"data", nullptr}
      };
      res.set_content(response.dump(), "application/json");
    }
  });
  
  // 启动服务器
  std::cout << "Server is running on http://localhost:8080" << std::endl;
  svr.listen("localhost", 8080);
  
  return 0;
}
