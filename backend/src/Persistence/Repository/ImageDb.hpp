#pragma once

#include "oatpp-sqlite/orm.hpp"

#include "Dto/Image.hpp"

#include OATPP_CODEGEN_BEGIN(DbClient)

class ImageDb : public oatpp::orm::DbClient {
public:

  ImageDb(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    oatpp::orm::SchemaMigration migration(executor);
    migration.addFile(1, DATABASE_MIGRATIONS "/001_images_init.sql");
    migration.migrate();

    auto version = executor->getSchemaVersion();
    OATPP_LOGD("ImageDb", "Migration - OK. Version=%lld.", version);
  }

  QUERY(createImage,
        "INSERT INTO images"
        "("
          "algo, start_date, end_date, "
          "data, height, width, iterations, "
          "reconstruction_time, user"
        ") VALUES "
        "("
          ":image.algo, :image.start_date, :image.end_date, "
          ":image.data, :image.height, :image.width, :image.iterations, "
          ":image.reconstruction_time, :image.user"
        ");",
        PARAM(oatpp::Object<Image>, image))

  QUERY(getImagesByUser,
        "SELECT * FROM images WHERE user=:user;",
        PARAM(oatpp::Int32, user))
};

#include OATPP_CODEGEN_END(DbClient)
