#define FUSE_USE_VERSION 26

#include <minerva-safefs-layer/minerva_layer.hpp>

#include <fuse.h>

static struct fuse_operations minerva_operations;

int main(int argc, char* argv[])
{
    minerva_operations.init = minerva_init;
    minerva_operations.opendir = minerva_opendir;
    minerva_operations.readdir = minerva_readdir;
    minerva_operations.getattr = minerva_getattr;
    minerva_operations.fgetattr = minerva_fgetattr;
    minerva_operations.access = minerva_access;
    minerva_operations.releasedir = minerva_releasedir;
    minerva_operations.write = minerva_write;
    minerva_operations.open = minerva_open;
    minerva_operations.read = minerva_read;            
    minerva_operations.mknod  = minerva_mknod;
    minerva_operations.mkdir = minerva_mkdir;
    minerva_operations.truncate = minerva_truncate;
    
    return fuse_main(argc, argv, &minerva_operations, NULL);
}