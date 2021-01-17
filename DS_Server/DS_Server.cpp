#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "service.grpc.pb.h"
#include "Database.cpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class DBServiceImpl final : public DBInterface::Service {

    DatabaseInterface* database = new Database();

    Status CreateTable(::grpc::ServerContext* context, const ::CreateTableRequest* request,
        ::CreateTableResponse* response) override {
        int keys_count = request->table().keys().size();
        std::string table_name = request->table().name();

        std::vector<std::string> keys;
        for (int i = 0; i < keys_count; i++) {
            keys.push_back(request->table().keys().Get(i));
        }

        bool result = database->CreateTable(table_name, keys);
        
        return result ? Status::OK : Status::Status(grpc::StatusCode::NOT_FOUND, "keker-chebureker");
    }

    Status DeleteTable(::grpc::ServerContext* context, const ::DeleteTableRequest* request,
        ::DeleteTableResponse* response) override {

        std::string name = request->table_name();

        bool result = database->DeleteTable(name);
        return result ? Status::OK : Status::Status(grpc::StatusCode::NOT_FOUND, database->GetErrorString());
    }

    Status GetFirstEntry(::grpc::ServerContext* context, const ::GetSeqEntryRequest* request,
        ::GetSeqEntryResponse* response) override {

        std::string table_name = request->table_name();
        std::string key_name = request->key_name();
        bool sort = request->sort();
        
        Entry entry = database->GetFirstEntry(table_name, key_name, sort);
        response->mutable_entry()->set_global_index(entry.global_index());
        response->mutable_entry()->set_value(entry.value());
        response->mutable_entry()->set_key_name(entry.key_name());
        response->mutable_entry()->set_key_value(entry.key_value());
        response->mutable_entry()->set_table_name(entry.table_name());
        response->mutable_entry()->set_sort(entry.sort());
        return entry.value() != "Error" ? 
            Status::OK : Status::Status(grpc::StatusCode::ABORTED, entry.table_name());
    }

    Status GetLastEntry(::grpc::ServerContext* context, const ::GetSeqEntryRequest* request,
        ::GetSeqEntryResponse* response) override {
        std::string table_name = request->table_name();
        std::string key_name = request->key_name();
        bool sort = request->sort();

        Entry entry = database->GetLastEntry(table_name, key_name, sort);
        response->mutable_entry()->set_global_index(entry.global_index());
        response->mutable_entry()->set_value(entry.value());
        response->mutable_entry()->set_key_name(entry.key_name());
        response->mutable_entry()->set_key_value(entry.key_value());
        response->mutable_entry()->set_table_name(entry.table_name());
        response->mutable_entry()->set_sort(entry.sort());

        return entry.value() != "Error" ?
            Status::OK : Status::Status(grpc::StatusCode::ABORTED, entry.table_name());
    }

    Status GetEntry(::grpc::ServerContext* context, const ::GetEntryRequest* request,
        ::GetEntryResponse* response) override {
        std::string table_name = request->table_name();
        std::string key_name = request->key_name();
        std::string key_value = request->key_value();

        Entry entry = database->GetEntry(table_name, key_name, key_value);
        response->mutable_entry()->set_global_index(entry.global_index());
        response->mutable_entry()->set_value(entry.value());
        response->mutable_entry()->set_key_name(entry.key_name());
        response->mutable_entry()->set_key_value(entry.key_value());
        response->mutable_entry()->set_table_name(entry.table_name());
        response->mutable_entry()->set_sort(entry.sort());

        return entry.value() != "Error" ?
            Status::OK : Status::Status(grpc::StatusCode::ABORTED, entry.table_name());
    }

    Status GetNextEntry(::grpc::ServerContext* context, const ::GetNextEntryRequest* request,
        ::GetNextEntryResponse* response) override {
        Entry entry = request->entry();
        Entry entry_next = database->GetNextEntry(entry);
        response->mutable_next_entry()->set_global_index(entry_next.global_index());
        response->mutable_next_entry()->set_value(entry_next.value());
        response->mutable_next_entry()->set_key_name(entry_next.key_name());
        response->mutable_next_entry()->set_key_value(entry_next.key_value());
        response->mutable_next_entry()->set_table_name(entry_next.table_name());
        response->mutable_next_entry()->set_sort(entry_next.sort());

        return entry_next.value() != "Error" ?
            Status::OK : Status::Status(grpc::StatusCode::ABORTED, entry_next.table_name());
    }

    Status GetPrevEntry(::grpc::ServerContext* context, const ::GetPrevEntryRequest* request,
        ::GetPrevEntryResponse* response) override {
        Entry entry = request->entry();
        Entry entry_next = database->GetPrevEntry(entry);
        response->mutable_prev_entry()->set_global_index(entry_next.global_index());
        response->mutable_prev_entry()->set_value(entry_next.value());
        response->mutable_prev_entry()->set_key_name(entry_next.key_name());
        response->mutable_prev_entry()->set_key_value(entry_next.key_value());
        response->mutable_prev_entry()->set_table_name(entry_next.table_name());
        response->mutable_prev_entry()->set_sort(entry_next.sort());

        return entry_next.value() != "Error" ?
            Status::OK : Status::Status(grpc::StatusCode::ABORTED, entry_next.table_name());
    }

    Status AddEntry(::grpc::ServerContext* context, const ::AddEntryRequest* request,
        ::AddEntryResponse* response) override {
        std::string value = request->value();
        std::string table_name = request->table_name();
        int keys_count = request->keys().size();
        std::vector<KeyValue> keys;
        for (int i = 0; i < keys_count; i++) {
            keys.push_back(request->keys().Get(i));
        }

        bool result = database->AddEntry(table_name, keys, value);
        return result ? Status::OK : Status::CANCELLED;
    }

    Status DeleteCurrentEntry(::grpc::ServerContext* context,
        const ::DeleteCurrentEntryRequest* request, 
        ::DeleteCurrentEntryResponse* response) override {
        Entry entry = request->entry();

        bool result = database->DeleteCurrentEntry(entry);
        return result ? Status::OK : Status::CANCELLED;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    DBServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}
