#include "App/App.h"
#include <fstream>
#include <algorithm>
#include <sqlite3.h>
#include <spdlog/spdlog.h>
#include <filesystem>

#include "App/json.h"


App::App() {
    m_manager.setMidiCallback([this](MidiDevice* device, MidiMessage msg) {
        this->handleMidiMessage(device, msg);

        if (this->m_midiCallback) this->m_midiCallback(device, msg);
    });
}


void App::setCurrentPage(const int page) { 
    std::lock_guard<std::mutex> lock(m_pageMutex);
    m_currentPage = page; 
}


void App::setMidiCallback(std::function<void(MidiDevice*, MidiMessage)> &&callback) {
    m_midiCallback = std::move(callback);
}

void App::onMidiBindingsChanged(std::function<void()> callback)
{
    m_midiBindingsManager.onMidiBindingsChanged(callback);
}


void App::onAudioListChanged(std::function<void()> callback)
{
    m_audioengine.onAudioListChanged(std::move(callback));
}


int App::addMidiBinding(MidiBinding binding) 
{
    return m_midiBindingsManager.addMidiBinding(binding);
}

void App::removeMidiBinding(const int id) 
{
    m_midiBindingsManager.removeMidiBinding(id);
}


const std::map<int, std::vector<MidiBinding>> &App::getMidiBindingPages() const
{
    return m_midiBindingsManager.getPages();
}


std::vector<MidiBinding*> App::getMidiBindingsForPage(int page) 
{
    return m_midiBindingsManager.getBindingsForPage(page);
}


MidiBindingBuilder App::midiBind(const std::string &deviceName)
{
    return MidiBindingBuilder(deviceName);
}


std::vector<MidiBinding *> App::getMidiBindings()
{
    return m_midiBindingsManager.getMidiBindings();
}


void App::handleMidiMessage(MidiDevice* device, MidiMessage msg) {
    m_midiBindingsManager.handleMidiMessage(this, device, msg);
}


nlohmann::json App::json() {
    std::map<std::string, std::vector<MidiBinding>> pages;

    std::transform(m_midiBindingsManager.getPages().begin(), m_midiBindingsManager.getPages().end(), std::inserter(pages, pages.begin()),
        [](const std::pair<int, std::vector<MidiBinding>>& page) {
            return std::make_pair(std::to_string(page.first), page.second);
        });

    nlohmann::json j = {
        { "bindings", pages },
        { "audio", nlohmann::json::parse(m_audioengine.json()) }
    };

    return j.dump();
}


bool App::load(std::string directory) {
    if (!std::filesystem::exists(directory)) {
        return false;
    }

    std::filesystem::path filepath = std::filesystem::path(directory) / "project.db";

    if (!std::filesystem::exists(filepath)) {
        return false;
    }

    if (loadBindings(filepath.string())) {
        return loadAudio(filepath.string());
    }

    return false;
}


bool App::save(std::string directory) {
    if (!std::filesystem::exists(directory)) {
        return false;
    }

    std::filesystem::path filepath = std::filesystem::path(directory) / "project.db";
    
    if (saveBindings(filepath.string())) {
        return saveAudio(filepath.string());
    }

    return false;
}


bool App::saveBindings(std::string filepath)
{
    spdlog::debug("Saving bindings to {}", filepath);

    sqlite3* db = nullptr;
    char*  err = nullptr;

    if (sqlite3_open(filepath.c_str(), &db) != SQLITE_OK) {
        spdlog::error("Could not open database: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    if (sqlite3_exec(db, "BEGIN;", nullptr, nullptr, &err) != SQLITE_OK) {
        spdlog::error("BEGIN failed: {}", err);
        sqlite3_free(err);
        sqlite3_close(db);
        return false;
    }

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS midi_bindings ("
        "  id        INTEGER PRIMARY KEY,"
        "  device    TEXT    NOT NULL,"
        "  event     INTEGER NOT NULL,"
        "  key_val   INTEGER NOT NULL,"
        "  audio_id  INTEGER NOT NULL,"
        "  to_page   INTEGER NOT NULL,"
        "  on_page   INTEGER NOT NULL"
        ");";

    if (sqlite3_exec(db, create_sql, nullptr, nullptr, &err) != SQLITE_OK) {
        spdlog::error("CREATE TABLE failed: {}", err);
        sqlite3_free(err);
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return false;
    }

    const char* insert_sql =
        "INSERT OR REPLACE INTO midi_bindings "
        "(id, device, event, key_val, audio_id, to_page, on_page) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* bindStmt = nullptr;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &bindStmt, nullptr) != SQLITE_OK) {
        spdlog::error("Failed to prepare INSERT: {}", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return false;
    }

    // Bind & execute for every binding
    auto pages = getMidiBindingPages();
    for (auto& [pageIdx, vec] : pages) {
        for (auto& binding : vec) {
            sqlite3_bind_int   (bindStmt, 1, binding.id);
            sqlite3_bind_text  (bindStmt, 2, binding.deviceName.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int   (bindStmt, 3, static_cast<int>(binding.eventType));
            sqlite3_bind_int   (bindStmt, 4, binding.key);
            sqlite3_bind_int   (bindStmt, 5, binding.audioId);
            sqlite3_bind_int   (bindStmt, 6, binding.toPage);
            sqlite3_bind_int   (bindStmt, 7, binding.onPage);

            if (sqlite3_step(bindStmt) != SQLITE_DONE) {
                spdlog::error("INSERT failed: {}", sqlite3_errmsg(db));
                sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
                sqlite3_close(db);
                return false;
            }
            sqlite3_reset(bindStmt);
        }
    }

    sqlite3_finalize(bindStmt);

    // COMMIT TRANSACTION
    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) {
        spdlog::error("COMMIT failed: {}", err);
        sqlite3_free(err);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    spdlog::debug("Bindings saved successfully ({} entries)", pages.size());

    return true;
}


bool App::saveAudio(std::string filepath)
{
    spdlog::debug("Saving audio to {}", filepath);

    std::vector<PlayerEntry> list = m_audioengine.list();

    sqlite3* db  = nullptr;
    char*    err = nullptr;

    if (sqlite3_open(filepath.c_str(), &db) != SQLITE_OK) {
        spdlog::error("Could not open database: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    // Wrap in a transaction
    if (sqlite3_exec(db, "BEGIN;", nullptr, nullptr, &err) != SQLITE_OK) {
        spdlog::error("BEGIN failed: {}", err);
        sqlite3_free(err);
        sqlite3_close(db);
        return false;
    }

    // 1) Create the audio_entries table
    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS audios ("
        "  id         INTEGER PRIMARY KEY,"
        "  file       TEXT    NOT NULL,"
        "  volume     REAL    NOT NULL,"
        "  start_time REAL    NOT NULL,"
        "  end_time   REAL    NOT NULL,"
        "  looped     INTEGER NOT NULL,"  // store bool as 0/1
        "  fade_in    REAL    NOT NULL,"
        "  fade_out   REAL    NOT NULL,"
        "  speed      REAL    NOT NULL"
        ");";

    if (sqlite3_exec(db, create_sql, nullptr, nullptr, &err) != SQLITE_OK) {
        spdlog::error("CREATE TABLE failed: {}", err);
        sqlite3_free(err);
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return false;
    }

    // 2) Prepare the upsert statement
    const char* insert_sql =
        "INSERT OR REPLACE INTO audios "
        "(id, file, volume, start_time, end_time, looped, fade_in, fade_out, speed) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        spdlog::error("Failed to prepare INSERT: {}", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return false;
    }

    // 3) Bind & execute for each PlayerEntry
    for (const auto& entry : list) {
        sqlite3_bind_int   (stmt, 1, static_cast<int>(entry.id));
        sqlite3_bind_text  (stmt, 2,
            entry.info.file.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 3, entry.info.settings.volume);
        sqlite3_bind_double(stmt, 4, entry.info.settings.start_time);
        sqlite3_bind_double(stmt, 5, entry.info.settings.end_time);
        sqlite3_bind_int   (stmt, 6, entry.info.settings.looped ? 1 : 0);
        sqlite3_bind_double(stmt, 7, entry.info.settings.fade_in);
        sqlite3_bind_double(stmt, 8, entry.info.settings.fade_out);
        sqlite3_bind_double(stmt, 9, entry.info.settings.speed);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            spdlog::error("INSERT failed: {}", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            sqlite3_close(db);
            return false;
        }
        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);

    // 4) Commit transaction
    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) {
        spdlog::error("COMMIT failed: {}", err);
        sqlite3_free(err);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    spdlog::debug("Audio saved successfully ({} entries)", list.size());


    return false;
}


bool App::loadBindings(std::string filepath)
{
    spdlog::debug("Loading bindings from {}", filepath);

    sqlite3* db = nullptr;
    char*  err = nullptr;

    if (sqlite3_open(filepath.c_str(), &db) != SQLITE_OK) {
        spdlog::error("Could not open database: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS midi_bindings ("
        "  id        INTEGER PRIMARY KEY,"
        "  device    TEXT    NOT NULL,"
        "  event     INTEGER NOT NULL,"
        "  key_val   INTEGER NOT NULL,"
        "  audio_id  INTEGER NOT NULL,"
        "  to_page   INTEGER NOT NULL,"
        "  on_page   INTEGER NOT NULL"
        ");";

    if (sqlite3_exec(db, create_sql, nullptr, nullptr, &err) != SQLITE_OK) {
        spdlog::error("CREATE TABLE failed: {}", err);
        sqlite3_free(err);
        sqlite3_close(db);
        return false;
    }

    const char* select_sql =
        "SELECT "
        "   id, device, event, key_val, audio_id, to_page, on_page " 
        "FROM midi_bindings;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        spdlog::error("SELECT failed: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    std::map<int, std::vector<MidiBinding>> pages;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        MidiBinding binding;
        binding.id = sqlite3_column_int(stmt, 0);
        binding.deviceName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        binding.eventType = static_cast<MidiMessage::Type>(sqlite3_column_int(stmt, 2));
        binding.key = sqlite3_column_int(stmt, 3);
        binding.audioId = sqlite3_column_int(stmt, 4);
        binding.toPage = sqlite3_column_int(stmt, 5);
        binding.onPage = sqlite3_column_int(stmt, 6);

        pages[binding.onPage].emplace_back(binding);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);


    m_midiBindingsManager.setMidiBindings(pages);

    return true;
}


bool App::loadAudio(std::string filepath)
{
    spdlog::debug("Loading audio from {}", filepath);

    sqlite3* db = nullptr;
    char*  err = nullptr;

    if (sqlite3_open(filepath.c_str(), &db) != SQLITE_OK) {
        spdlog::error("Could not open database: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    const char* select_sql =
        "SELECT "
        "   id, file, start_time, end_time, looped, fade_in, fade_out, speed " 
        "FROM audios;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        spdlog::error("SELECT failed: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    std::map<int, AudioBuilder> audios;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        float start_time = sqlite3_column_double(stmt, 2);
        float end_time = sqlite3_column_double(stmt, 3);
        bool looped = sqlite3_column_int(stmt, 4);
        float fade_in = sqlite3_column_double(stmt, 5);
        float fade_out = sqlite3_column_double(stmt, 6);
        float speed = sqlite3_column_double(stmt, 7);

        AudioBuilder builder;
        builder.set_file(path);
        builder.set_start(start_time);
        builder.set_end(end_time);
        builder.set_loop(looped);
        builder.set_fade_in(fade_in);
        builder.set_fade_out(fade_out);
        builder.set_speed(speed);
        
        audios[id] = builder;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    this->audioEngine().load(audios);
    return true;
}
