#pragma once

#include <string>

#include <storages/mongo/bulk.hpp>
#include <storages/mongo/cursor.hpp>
#include <storages/mongo/operations.hpp>
#include <storages/mongo/stats.hpp>
#include <storages/mongo/write_result.hpp>
#include <tracing/span.hpp>

namespace storages::mongo::impl {

class CollectionImpl {
 public:
  virtual ~CollectionImpl() = default;

  const std::string& GetDatabaseName() const;
  const std::string& GetCollectionName() const;

  virtual size_t Execute(const operations::Count&) const = 0;
  virtual size_t Execute(const operations::CountApprox&) const = 0;
  virtual Cursor Execute(const operations::Find&) const = 0;
  virtual WriteResult Execute(const operations::InsertOne&) = 0;
  virtual WriteResult Execute(const operations::InsertMany&) = 0;
  virtual WriteResult Execute(const operations::ReplaceOne&) = 0;
  virtual WriteResult Execute(const operations::Update&) = 0;
  virtual WriteResult Execute(const operations::Delete&) = 0;
  virtual WriteResult Execute(const operations::FindAndModify&) = 0;
  virtual WriteResult Execute(const operations::FindAndRemove&) = 0;
  virtual WriteResult Execute(operations::Bulk&&) = 0;
  virtual Cursor Execute(const operations::Aggregate&) = 0;

 protected:
  CollectionImpl(std::string&& database_name, std::string&& collection_name);

  tracing::Span MakeSpan(const std::string& name) const;

 private:
  const std::string database_name_;
  const std::string collection_name_;
};

}  // namespace storages::mongo::impl
