/* Модуль BreezeCAD — точка входа.
 *
 * Весь договор с программой — в sdk/bz_module.h:
 *  - экспортировать bz_module_entry() и вернуть таблицу своих функций;
 *  - в init() проверить версию ABI программы и запомнить её таблицу;
 *  - читать сборку через host->component_*; менять документы — только
 *    через host->execute() (это обычные шаги отмены Ctrl+Z);
 *  - в shutdown() всё освободить и забыть указатели программы.
 *
 * Модуль можно писать и на C++: переименуйте файл в .cpp и поправьте
 * список исходников в CMakeLists.txt. Исключения через границу не бросать.
 */
#include "bz_module.h"

#include <stddef.h>
#include <stdio.h>

static const BzHostApi* host = NULL;

static int32_t module_init(const BzHostApi* h) {
    /* Программа другой версии ABI или слишком старая — не запускаемся. */
    if (!h || h->abi_version != BZ_MODULE_ABI_VERSION || h->struct_size < offsetof(BzHostApi, request_main)) return BZ_EFAIL;
    host = h;

    char    line[160];
    int32_t count = 0;
    if (host->component_count(&count) == BZ_OK)
        snprintf(line, sizeof line, "Модуль example: в сборке компонентов — %d", (int)count);
    else
        snprintf(line, sizeof line, "Модуль example запущен (BreezeCAD %s), сборка не открыта", host->host_version());
    host->log(BZ_LOG_INFO, line);

    /* Команды execute() — только на главном потоке, в main_tick. */
    if (BZ_HAS(host, request_main)) host->request_main();
    return BZ_OK;
}

static void module_main_tick(void) {
    const char* reply = NULL;
    if (!host || !BZ_HAS(host, execute)) return;
    /* Пример команды. Список всех: {"command": "commands.list"}. */
    if (host->execute("{\"command\": \"app.documents\"}", &reply) == BZ_OK && reply) {
        char line[400];
        snprintf(line, sizeof line, "Модуль example: открытые документы — %.300s", reply);
        host->log(BZ_LOG_INFO, line);
    }
}

static void module_shutdown(void) {
    if (host) host->log(BZ_LOG_INFO, "Модуль example: выключен");
    host = NULL;
}

static const BzModuleApi api = {BZ_MODULE_ABI_VERSION, sizeof(BzModuleApi), module_init, module_shutdown, module_main_tick};

BZ_MODULE_EXPORT const BzModuleApi* bz_module_entry(void) { return &api; }
