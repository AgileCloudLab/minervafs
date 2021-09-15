#include "registry.hpp"

#include <tartarus/writers.hpp>
#include <tartarus/readers.hpp>

#include <filesystem>

#include <sstream>
#include <iostream>

#include <stdexcept>

namespace minerva
{

    registry::registry(const nlohmann::json& config)
    {
        std::cout << "CREATING REGISTRY\n"; 

        if (config.find("fileout_path") == config.end())             
        {
            throw std::runtime_error("Missing fileout");
            // TODO: Throw exception
        }

        if (
            config.find("index_path") == config.end())             
        {
            throw std::runtime_error("index");
            // TODO: Throw exception
        }

        if (
            config.find("major_group_length") == config.end() ||
            config.find("minor_group_length") == config.end())             
        {
            throw std::runtime_error("major minor config");
            // TODO: Throw exception
        }        

        if (config.find("fileout_path") == config.end() ||
            config.find("index_path") == config.end() ||
            config.find("major_group_length") == config.end() ||
            config.find("minor_group_length") == config.end())             
        {
            throw std::runtime_error("Missing config");
            // TODO: Throw exception
        }
        else
        {
            m_fileout_path = config["fileout_path"].get<std::string>();
            m_index_path = config["index_path"].get<std::string>();
            m_major_length = config["major_group_length"].get<size_t>();
            m_minor_length = config["minor_group_length"].get<size_t>();            
        }

        if (config.contains("versioning"))
        {
            auto version_config = config["versioning"].get<nlohmann::json>(); 
            if (version_config.contains("version_path"))
            {
                m_version = minerva::version(version_config["version_path"].get<std::string>());
                m_versioning = true; 
            }
            else
            {
                m_versioning = false; 
            }
        }

        if (config.contains("compression"))
        {          
            auto compression_config = config["compression"].get<nlohmann::json>();

            if (!compression_config.contains("uncompressed_size"))
            {
                // todo throw error
            }
            
            if (!compression_config.contains("algorithm"))
            {
                // Todo throw error
            }

            if (!compression_config.contains("configuration"))
            {
                // TODO throw error
            }

            m_uncompressed_size = compression_config["uncompressed_size"].get<size_t>();
            m_compression_algorithm = compression_config["algorithm"].get<ananke::algorithm>();
            m_compression_config = compression_config["configuration"].get<nlohmann::json>(); 
                       
            std::cout << "compression on" << std::endl;            
            m_compression = true; 
        }
        else
        {
            std::cout << "compression off" << std::endl;            
            m_compression = false; 
        }
        
        if (config.find("in_memory") == config.end())
        {
            m_in_memory = false; 
        }
        else
        {
            m_in_memory = config["in_memory"].get<bool>(); 
        }
    }

    void registry::write_file(const std::string& path, const std::vector<uint8_t>& data)
    {

        if (m_versioning)
        {
            // TODO check version path
            m_version.store_version(path, data);
        }
        // If versioning is enabled we overwrite the old version with the new version in the placeholder directory 
        tartarus::writers::vector_disk_writer(path, data);
        
    }


    std::vector<uint8_t> registry::load_file(const std::string& path)
    {

        return tartarus::readers::vector_disk_reader(m_fileout_path + "/" + path);
        
    }

    void registry::store_bases(const std::map<std::vector<uint8_t>, std::vector<uint8_t>>& fingerprint_basis)
    {

        std::map<std::vector<uint8_t>, std::vector<uint8_t>> to_store;

        for (auto it = fingerprint_basis.begin(); it != fingerprint_basis.end(); ++it)
        {
            if (!basis_exists(it->first))
            {
                to_store[it->first] = it->second; 
            }
        }

        for (auto it = to_store.begin(); it != to_store.end(); ++it)
        {
            std::filesystem::path basis_path(get_basis_path(it->first));
            if (!std::filesystem::exists(basis_path.parent_path()))
            {
                std::filesystem::create_directories(basis_path.parent_path());
            }
            
            auto basis = it->second;
            if (m_compression)
            {
                auto res = ananke::compress(m_compression_algorithm, m_compression_config, basis);
                basis = res.second; 
            }

            tartarus::writers::vector_disk_writer(basis_path.string(), basis);
            if (m_in_memory)
            {
                m_in_memory_registry[it->first] = 1; 
            }
        }
    }

    void registry::load_bases(std::map<std::vector<uint8_t>, std::vector<uint8_t>>& fingerprint_basis)
    {
        for (auto it = fingerprint_basis.begin(); it != fingerprint_basis.end(); ++it)
        {
            auto basis = tartarus::readers::vector_disk_reader(get_basis_path(it->first));
            if (m_compression)
            {
                basis = ananke::decompress(m_compression_algorithm, m_compression_config, basis, m_uncompressed_size); 
            }
            
            fingerprint_basis[it->first] = basis;
        }
    }

    void registry::delete_bases(std::vector<std::vector<uint8_t>>& fingerprints)
    {
        for (const auto& fingerprint : fingerprints)
        {
            std::filesystem::remove(get_basis_path(fingerprint)); 
        }
    }
    
    bool registry::basis_exists(const std::vector<uint8_t>& fingerprint)
    {
        if (m_in_memory)
        {
            return m_in_memory_registry.find(fingerprint) != m_in_memory_registry.end() ? true : false;
        }

        return std::filesystem::exists(get_basis_path(fingerprint)) ? true : false;
    }

    std::string registry::convert_fingerprint_to_string(const std::vector<uint8_t>& fingerprint)
    {
        std::stringstream ss;

        for (const auto& elm : fingerprint)
        {
            ss << std::hex << (int) elm;
        }
        
        return ss.str();
    }

    std::string registry::get_basis_path(const std::vector<uint8_t>& fingerprint)
    {
        std::string fingerprint_str = convert_fingerprint_to_string(fingerprint);

        return m_index_path + "/" + fingerprint_str.substr(0, m_major_length) + "/"
            + fingerprint_str.substr(0 + m_major_length, m_minor_length) + "/" + fingerprint_str;   
    }
}
