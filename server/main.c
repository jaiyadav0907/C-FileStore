#include "http_server.h"
#include "object_store.h"
#include "logger.h"

int main() {
    initLogger("server.log");
    ensureStorageDirExists();
    startHttpServer();
    return 0;
}