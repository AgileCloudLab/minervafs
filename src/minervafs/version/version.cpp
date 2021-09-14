#include "version.hpp"

#include "../utils.hpp"

#include <tartarus/writers.hpp>
#include <tartarus/readers.hpp>

#include <fmt/core.h>

#include <filesystem>

#include <cstdlib>

namespace minerva
{

    version::version(const std::string& version_path)
    {
        m_version_path = version_path;
        
        if (version_path.size() > 0 && version_path.substr(version_path.size() - 1) != "/")
        {
            m_version_path = fmt::format("{}/", m_version_path);
        }

        if (!std::filesystem::exists(m_version_path))
        {
            create_directory(m_version_path);
        }
    }

    void version::store_version(const std::string& file_path, const std::vector<uint8_t>& data)
    {
        std::string full_path;

        if (file_path.at(0) == '/')
        {
            full_path = fmt::format("{}{}", m_version_path, file_path.substr(1, file_path.length())); 
        }
        
        if (!std::filesystem::exists(full_path))
        {
            create_directory(full_path); 
//            std::filesystem::create_directories(fmt::format("{}{}", m_version_path, file_path));
        }
        
        std::string write_path = create_version_path(file_path);

        fmt::print("HER\n"); 
        tartarus::writers::vector_disk_writer(write_path, data);
        fmt::print("HER2\n");         
    }

    std::vector<uint8_t> version::load_version(std::string file_path)
    {

        std::string read_path = current_version_path(file_path);
        return tartarus::readers::vector_disk_reader(read_path);

    }

    std::string version::current_version_path(const std::string& path)
    {

        if (!std::filesystem::exists(m_version_path + path))
        {

        }

        int current = 0;

        for (const auto& entry : std::filesystem::directory_iterator(m_version_path + path))
        {
            (void) entry;
            ++current;
        }

        if (path.at(path.size() - 1) != '/')
        {
            return m_version_path + path + "/" + std::to_string(current);
        }
        else
        {
            return m_version_path + path + std::to_string(current);
        }
    }

    uint32_t version::next_version(const std::string& path)
    {
        if (!std::filesystem::exists(m_version_path + path))
        {
            return 1;
        }

        uint32_t newest = 1;

        for (const auto& entry: std::filesystem::directory_iterator(m_version_path + path))
        {
            (void) entry;
            ++newest;
        }

        return newest;
    }

    std::string version::create_version_path(const std::string& path, const uint32_t version)
    {

        std::string base = "{}{}";

        if (path.at(0) == '/')
        {
            base = fmt::format(base, m_version_path, path.substr(1, path.length())); 
        }
        else
        {
            base = fmt::format(base, m_version_path, path);             
        }
        
        
        std::string pattern = "{}{}";
        
        if (base.at(base.length() - 1) != '/')
        {
            pattern = "{}/{}";
        }

        return fmt::format(pattern, base, std::to_string(version)); 
    }

    std::string version::create_version_path(const std::string& path)
    {
        return create_version_path(path, next_version(path)); 
    }    

    bool version::is_first_version(const std::string& path)
    {
        // If exists it is not the first version 
        return !std::filesystem::exists(path);
    }    
}
