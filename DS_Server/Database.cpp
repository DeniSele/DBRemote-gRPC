#include <map>
#include <vector>
#include <iostream>
#include <shared_mutex>

#include <fstream>
#include <sstream>
#include "DatabaseInterface.cpp"
#include <algorithm>
#include <windows.h>
#include<tchar.h>

class Database : public DatabaseInterface {

	class Table
	{
		struct IndexedEntry
		{
			int index;
			Entry entry;

			IndexedEntry() {
				index = 0;
				entry = Entry();
			}
		};

		std::string table_name = "New table";

		// index:value
		std::map<int, std::string> main_table;
		std::map<int, std::vector<std::pair<std::string, std::string>>> main_keys_table;

		// key_name: hash_table<key_value, indexes>
		std::map<std::string, std::map<std::string, std::vector<int>>> keys_map;

		std::string error_message = "undefined_error";
		Entry errorEntry;
		IndexedEntry errorIndexedEntry;

		int global_index;

		std::shared_timed_mutex tableMutex;

	public:

		void Save() {
			std::ofstream fout;
			fout.open(table_name + ".data");
			//header - names of keys from keys_map
			std::string header = "";
			for (auto const& x : keys_map)
			{
				header += "\"" + x.first + "\",";
			}
			header += "\n";
			fout << header;

			// record global_index,"key_name1":"key_value1","key_name2":"key_value2",key_name1:"key_value1","value"
			// key_value can be empty it will look like global_index, "key_name1":"key_value1", "key_name2":"","value"
			for (auto const& key : main_table)
			{
				//value
				std::string result_string = "";
				std::string value = key.second;

				//global_index component
				int global_index = key.first;
				result_string += std::to_string(global_index) + ",";

				//iterating through key_map because not all key_values can be presented so
				//it is nessesary to fill with "" in these cases
				for (auto const& key_name : keys_map)
				{
					//flag if this key_value is presented to this global_index
					bool contains = false;
					for (auto const& key_name_key_value : main_keys_table[global_index])
					{
						//if presented
						if (key_name_key_value.first == key_name.first) {
							//fill key_name and key_value
							result_string += "\"" + key_name_key_value.first + ":" + key_name_key_value.second + "\",";
							//switch flag key_name is in the vector
							contains = true;
							break;
						}

					}

					//there is no key_value for such key_name so fill with ""
					if (!contains) {
						result_string += "\"" + key_name.first + ":\",";
					}
				}

				//write result string
				result_string += "\"" + value + "\"\n";
				fout << result_string;
			}

			fout.close();
		}

		void Load(const std::string table_name_param) {
			//filling main_table and main_keys_table

			std::ifstream fin;
			fin.open(table_name_param);
			table_name = table_name_param.substr(0, table_name_param.size() - 5);
			//header+
			std::string header = "";
			std::getline(fin, header);
			auto key_names = split(header, ',');
			for (int i = 0; i < key_names.size(); i++) {
				key_names[i] = key_names[i].substr(1, key_names[i].size() - 2);
			}

			//reading records 1by1
			std::string record;
			while (std::getline(fin, record))
			{
				//all fields
				auto fields = split(record, ',');


				// global index - 1 field
				int index = std::stoi(fields[0]);

				// value - last field
				std::string value = fields[fields.size() - 1];
				value = value.substr(1, value.size() - 2);

				//deleting index and value to leave in fields vector only names and values of keys
				fields.erase(fields.begin());
				fields.erase(--fields.end());

				std::vector<std::pair<std::string, std::string>> key_names_key_values;
				for (auto const& key_name_key_value : fields)
				{
					//splitting to key_name and key_value(they are separated by :)
					auto key_name_key_value_vector = split(key_name_key_value, ':');

					//removing extra "
					auto key_name = key_name_key_value_vector[0];
					key_name = key_name.substr(1, key_name.size() - 1);

					//removing extra "
					auto key_value = key_name_key_value_vector[1];
					key_value = key_value.substr(0, key_value.size() - 1);

					if (key_value.size() != 0) {
						//making pair key_name_key_value and pushing to vector of key_name_key_values
						key_names_key_values.push_back(std::make_pair(key_name, key_value));
					}

				}

				//fill main keys_table and main_table with data from file
				main_keys_table[index] = key_names_key_values;
				main_table[index] = value;
			}

			////CHECKCKCKCK

			//filling key_map with main_keys_table
			for (auto const& index_key_name_key_value : main_keys_table)
			{
				//get global index
				int global_index = index_key_name_key_value.first;
				for (auto const& key_name_key_value : index_key_name_key_value.second)
				{
					//is key_name in keys_map?
					auto it_key_name = keys_map.find(key_name_key_value.first);
					if (it_key_name == keys_map.end()) {
						//nah

						//need to create map of key_value_indexes
						std::map<std::string, std::vector<int>> key_value_indexes;
						//need to create vector of indexes
						std::vector<int> indexes;
						//filling vector with global_index
						indexes.push_back(global_index);
						//filling map with key_name and just created vector of indexes
						key_value_indexes[key_name_key_value.second] = indexes;
						//pushing key_name_map_of_key_values
						keys_map[key_name_key_value.first] = key_value_indexes;
					}
					else {
						//yeah

						//key_values map
						auto key_value_indexes = it_key_name->second;
						auto it_key_values = key_value_indexes.find(key_name_key_value.second);

						//is this key_value in map of key_value_indexes
						if (it_key_values == key_value_indexes.end())
						{
							//nah

							std::vector<int> indexes;
							//filling vector with global_index
							indexes.push_back(global_index);
							//filling map with key_name and just created vector of indexes
							it_key_name->second[key_name_key_value.second] = indexes;


						}
						else {
							//yeah

							//to existing key_value pushing to vector global_index
							it_key_values->second.push_back(global_index);
						}

					}
				}

			}

			for (auto const& key_name : key_names)
			{
				//is key_name in keys_map?
				auto it_key_name = keys_map.find(key_name);
				if (it_key_name == keys_map.end()) {
					//nah

					//need to create map of key_value_indexes
					std::map<std::string, std::vector<int>> key_value_indexes;
					//pushing key_name_map_of_key_values
					keys_map[key_name] = key_value_indexes;
				}
			}

			fin.close();
		}

		Table() { };

		Table(const std::string name, const std::vector<std::string> keys)
		{
			table_name = name;
			global_index = 1000;

			for (std::string key : keys) {
				std::map<std::string, std::vector<int>> empty_map;
				keys_map[key] = empty_map;
			}

			errorEntry.set_value("Error");
			errorEntry.set_table_name("Error message");

			errorIndexedEntry.index = -1;
			errorIndexedEntry.entry = errorEntry;
		};

		~Table() { };


		Entry GetEntry(const std::string key_name, const std::string key_value) {
			std::shared_lock<std::shared_timed_mutex> readerLock(tableMutex);
			return GetIndexedEntry(key_name, key_value).entry;
		}

		Entry GetNextEntry(const Entry entry) {
			std::shared_lock<std::shared_timed_mutex> readerLock(tableMutex);

			if (HasKey(entry.key_name())) {
				auto iterator = keys_map[entry.key_name()].find(entry.key_value());
				if (iterator != keys_map[entry.key_name()].end()) {
					
					//Check same key names first
					if ((*iterator).second.size() > 1) {
						std::vector<int> indexes = (*iterator).second;
						if (indexes[indexes.size() - 1] != entry.global_index()) {
							auto itemIndex = std::find(indexes.begin(), indexes.end(), entry.global_index());
							itemIndex++;
							int new_index = (*itemIndex);
							return ReturnIndexedEntry(new_index, main_table[new_index], entry.key_name(), entry.key_value(), entry.sort()).entry;
						}
					}

					iterator++;
					if (iterator == keys_map[entry.key_name()].end())
						iterator = keys_map[entry.key_name()].begin();

					int index = (*iterator).second[0];
					std::string key_value = (*iterator).first;
					std::string value = main_table[index];
					
					return ReturnIndexedEntry(index, value, entry.key_name(), key_value, entry.sort()).entry;
				}

				errorEntry.set_table_name("Error. No key value found with key [" + entry.key_value() + "]");
				return errorEntry;
			}

			errorEntry.set_table_name("Error. No key found with name [" + entry.key_name() + "]");
			return errorEntry;
		}

		Entry GetPrevEntry(const Entry entry) {
			std::shared_lock<std::shared_timed_mutex> readerLock(tableMutex);

			if (HasKey(entry.key_name())) {
				auto iterator = keys_map[entry.key_name()].find(entry.key_value());
				if (iterator != keys_map[entry.key_name()].end()) {

					//Check same key names first
					if ((*iterator).second.size() > 1) {
						std::vector<int> indexes = (*iterator).second;
						if (indexes[0] != entry.global_index()) {
							auto itemIndex = std::find(indexes.begin(), indexes.end(), entry.global_index());
							itemIndex--;
							int new_index = (*itemIndex);
							return ReturnIndexedEntry(new_index, main_table[new_index], entry.key_name(), entry.key_value(), entry.sort()).entry;
						}
					}

					if (iterator == keys_map[entry.key_name()].begin())
						iterator = keys_map[entry.key_name()].end();
					iterator--;

					int index = (*iterator).second[0];
					std::string key_value = (*iterator).first;
					std::string value = main_table[index];

					return ReturnIndexedEntry(index, value, entry.key_name(), key_value, entry.sort()).entry;
				}

				errorEntry.set_table_name("Error. No key value found with key [" + entry.key_value() + "]");
				return errorEntry;
			}

			errorEntry.set_table_name("Error. No key found with name [" + entry.key_name() + "]");
			return errorEntry;
		}

		bool CreateEntry(const std::vector<KeyValue> keys, const std::string value) {
			std::lock_guard<std::shared_timed_mutex> writerLock(tableMutex);

			for (KeyValue key_pair : keys) {
				if (!HasKey(key_pair.name())) {
					error_message = "Error. No key found with name [" + key_pair.name() + "]";
					return false;
				}
			}

			for (KeyValue key_pair : keys) {
				if (keys_map[key_pair.name()].find(key_pair.value()) == keys_map[key_pair.name()].end()) {
					keys_map[key_pair.name()][key_pair.value()] = std::vector<int>();
				}
				keys_map[key_pair.name()][key_pair.value()].push_back(global_index);
				main_keys_table[global_index].push_back(std::make_pair(key_pair.name(), key_pair.value()));
			}

			main_table[global_index] = value;
			main_keys_table[global_index] = std::vector<std::pair<std::string, std::string>>();

			global_index++;
			return true;
		}

		bool DeleteEntry(const Entry entry) {
			std::lock_guard<std::shared_timed_mutex> writerLock(tableMutex);

			if (!HasKey(entry.key_name())) {
				error_message = "Error. No key found with name [" + entry.key_name() + "]";
				return false;
			}

			if (keys_map[entry.key_name()].find(entry.key_value()) != keys_map[entry.key_name()].end()) {
				IndexedEntry entryToDelete = GetIndexedEntry(entry.key_name(), entry.key_value());
				main_table.erase(entryToDelete.index);
				for (std::pair<std::string, std::string> key_pair : main_keys_table[entryToDelete.index]) {
					std::vector<int> indexes = keys_map[key_pair.first][key_pair.second];
					indexes.erase(std::remove(indexes.begin(), indexes.end(), entryToDelete.index), indexes.end());
					
					if (indexes.size() == 0) {
						keys_map[key_pair.first].erase(key_pair.second);
					}
					else {
						keys_map[key_pair.first][key_pair.second] = indexes;
					}
				}

				return true;
			}

			error_message = "Error. No key value found with name [" + entry.key_value() + "]";
			return false;
		}

		Entry GetEntrySorted(const bool from_front, const bool sort, const std::string key_name) {
			std::shared_lock<std::shared_timed_mutex> readerLock(tableMutex);

			std::vector<std::string> keys_values;
			for (auto it = keys_map[key_name].begin(); it != keys_map[key_name].end(); ++it) {
				keys_values.push_back(it->first);
			}

			if (keys_values.empty()) {
				errorEntry.set_table_name("Error. No keys values found with key [" + key_name + "]");
				return errorEntry;
			}

			std::sort(keys_values.begin(), keys_values.end());
			std::string key_value = (sort ? !from_front : from_front) ? keys_values.front() : keys_values.back();
			int index = keys_map[key_name][key_value][0];
			std::string value = main_table[index];
			return ReturnIndexedEntry(index, value, key_name, key_value, sort).entry;
		}

		std::string GetErrorMessage() {
			return error_message;
		}

	private:
		IndexedEntry GetIndexedEntry(const std::string key_name, const std::string key_value) {
			if (HasKey(key_name)) {
				if (keys_map[key_name].find(key_value) != keys_map[key_name].end()) {
					if (keys_map[key_name][key_value].size() > 0) {
						int index = keys_map[key_name][key_value][0];
						std::string value = main_table[index];
						return ReturnIndexedEntry(index, value, key_name, key_value, true);
					}

					errorIndexedEntry.entry.set_table_name("Error. No value found with key name [" + key_value + "]");
					return errorIndexedEntry;
				}

				errorIndexedEntry.entry.set_table_name("Error. No key value found with key [" + key_value + "]");
				return errorIndexedEntry;
			}

			errorIndexedEntry.entry.set_table_name("Error. No key found with name [" + key_name + "]");
			return errorIndexedEntry;
		}

		IndexedEntry ReturnIndexedEntry(const int index, const std::string value, const std::string key_name, const std::string key_value, const bool sort) {
			Entry entry;
			entry.set_global_index(index);
			entry.set_value(value);
			entry.set_table_name(table_name);
			entry.set_key_name(key_name);
			entry.set_key_value(key_value);
			entry.set_sort(sort);

			IndexedEntry indexedEntry;
			indexedEntry.index = index;
			indexedEntry.entry = entry;

			return indexedEntry;
		}

		bool HasKey(const std::string key_name) {
			return keys_map.find(key_name) != keys_map.end();
		}

		std::vector<std::string> split(const std::string& s, char delim) {
			std::vector<std::string> elems;
			std::stringstream ss;
			ss.str(s);
			std::string item;
			while (std::getline(ss, item, delim)) {
				elems.push_back(item);
			}
			return elems;
		}
	};

	// all tables
	std::map<std::string, Table*> tables;

	std::shared_timed_mutex databaseMutex;

	std::string error_message = "undefined_error";
	Entry error_entry;

public:

	Database() {
		error_entry.set_value("Error");
	}

	void Save() override  {
		for (auto table : tables) {
			table.second->Save();
		}
	}

	void Load() override {
		std::string path = "D:\\DatabaseDS\\Debug";
		auto table_names = get_all_files_names_within_folder(path);
		for (auto const& table_name_local : table_names) {
			auto table = new Table();
			table->Load(table_name_local);
			tables[table_name_local.substr(0, table_name_local.size() - 5)] = table;
		}
	}

	bool CreateTable(const std::string name, const std::vector<std::string> keys) override
	{
		std::lock_guard<std::shared_timed_mutex> writerLock(databaseMutex);

		if (HasTable(name)) {
			return false;
		}

		tables[name] = new Table(name, keys);
		return true;
	}

	bool DeleteTable(const std::string name) override
	{
		std::lock_guard<std::shared_timed_mutex> writerLock(databaseMutex);

		if (!HasTable(name)) {
			error_message = "Error. Table was not found.";
			return false;
		}
		tables.erase(name);
		return true;
	}

	Entry GetFirstEntry(const std::string name, const std::string key_name, const bool sort_order) override
	{
		std::shared_lock<std::shared_timed_mutex> readerLock(databaseMutex);

		if (!HasTable(name)) {
			error_message = "Error. Table was not found.";
			return error_entry;
		}

		return tables[name]->GetEntrySorted(true, sort_order, key_name);
	}

	Entry GetLastEntry(const std::string name, const std::string key_name, const bool sort_order) override
	{
		std::shared_lock<std::shared_timed_mutex> readerLock(databaseMutex);

		if (!HasTable(name)) {
			error_message = "Error. Table was not found.";
			return error_entry;
		}
		return tables[name]->GetEntrySorted(false, sort_order, key_name);
	}

	Entry GetEntry(const std::string name, const std::string key_name, const std::string key_value) override
	{
		std::shared_lock<std::shared_timed_mutex> readerLock(databaseMutex);

		if (!HasTable(name)) {
			error_message = "Error. Table was not found.";
			return error_entry;
		}
		return tables[name]->GetEntry(key_name, key_value);
	}

	Entry GetNextEntry(const Entry entry) override
	{
		std::shared_lock<std::shared_timed_mutex> readerLock(databaseMutex);
		return entry.sort() ? tables[entry.table_name()]->GetPrevEntry(entry) : tables[entry.table_name()]->GetNextEntry(entry);
	}

	Entry GetPrevEntry(const Entry entry) override
	{
		std::shared_lock<std::shared_timed_mutex> readerLock(databaseMutex);
		return entry.sort() ? tables[entry.table_name()]->GetNextEntry(entry) : tables[entry.table_name()]->GetPrevEntry(entry);
	}

	bool AddEntry(const std::string table_name, const std::vector<KeyValue> keys, const std::string value) override
	{
		std::shared_lock<std::shared_timed_mutex> readerLock(databaseMutex);

		if (!HasTable(table_name)) {
			error_message = "Error. Table was not found.";
			return false;
		}
		return tables[table_name]->CreateEntry(keys, value);
	}

	bool DeleteCurrentEntry(const Entry entry) override
	{
		std::shared_lock<std::shared_timed_mutex> readerLock(databaseMutex);

		bool result = tables[entry.table_name()]->DeleteEntry(entry);
		if (!result) {
			error_message = tables[entry.table_name()]->GetErrorMessage();
		}

		return result;
	}

	std::string GetErrorString() override {
		return error_message;
	}

private:

	bool HasTable(const std::string table_name) {
		return tables.find(table_name) != tables.end();
	}

	std::vector<std::string> get_all_files_names_within_folder(const std::string folder)
	{
		std::vector<std::string> names;
		std::string search_path = folder + "\\*.data*";
		WIN32_FIND_DATAA fd;
		HANDLE hFind = ::FindFirstFileA(search_path.c_str(), &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				// read all (real) files in current folder
				// , delete '!' read other 2 default folder . and ..
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					names.push_back(fd.cFileName);
				}
			} while (::FindNextFileA(hFind, &fd));
			::FindClose(hFind);
		}

		return names;
	}
};