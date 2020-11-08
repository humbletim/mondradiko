/**
 * @file client_main.cc
 * @author Marceline Cramer (cramermarceline@gmail.com)
 * @brief Entrypoint for a command-line client session.
 * @date 2020-10-24
 *
 * @copyright Copyright (c) 2020 Marceline Cramer
 *
 * This file is part of Mondradiko.
 *
 * Mondradiko is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mondradiko is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Mondradiko.  If not, see <https: //www.gnu.org/licenses/>.
 *
 */

#include <iostream>

#include "log/log.h"
#include "session/player_session.h"
#include "src/api_headers.h"

int main(int argc, const char* argv[]) {
    log_inf("Hello VR!");

    PHYSFS_init(argv[0]);

    try {
        player_session_run("127.0.0.1", 10555);
    } catch(const std::exception& e) {
        log_err("Mondradiko player session failed with message:");
        log_err(e.what());
        PHYSFS_deinit();
        return 1;
    }

    PHYSFS_deinit();
    return 0;
}
