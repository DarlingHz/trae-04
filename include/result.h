#pragma once

#include <string>
#include <optional>

 template <typename T>
 class Result {
 public:
   // 成功构造函数
   static Result<T> Success(const T& data) {
     return Result<T>(true, data, "");
   }
   
   // 错误构造函数
   static Result<T> Error(const std::string& error) {
     return Result<T>(false, std::nullopt, error);
   }
   
   // 判断是否成功
   bool IsSuccess() const {
     return success_;
   }
   
   // 获取数据
   const T& GetData() const {
     return data_.value();
   }
   
   // 获取错误信息
   const std::string& GetError() const {
     return error_;
   }
   
 private:
   // 私有构造函数
   Result(bool success, const std::optional<T>& data, const std::string& error)
       : success_(success), data_(data), error_(error) {
   }
   
   bool success_;
   std::optional<T> data_;
   std::string error_;
 };
 
 // 特化Result<void>模板
 template <> 
 class Result<void> {
 public:
   // 成功构造函数
   static Result<void> Success() {
     return Result<void>(true, "");
   }
   
   // 错误构造函数
   static Result<void> Error(const std::string& error) {
     return Result<void>(false, error);
   }
   
   // 判断是否成功
   bool IsSuccess() const {
     return success_;
   }
   
   // 获取错误信息
   const std::string& GetError() const {
     return error_;
   }
   
 private:
   // 私有构造函数
   Result(bool success, const std::string& error)
       : success_(success), error_(error) {
   }
   
   bool success_;
   std::string error_;
 };
