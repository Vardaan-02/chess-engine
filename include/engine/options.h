// #pragma once

// /**
//  * @file options.h
//  * @brief Defines a structure for tunable engine parameters.
//  *
//  * These options can be modified at runtime by the UCI 'setoption' command,
//  * allowing for flexible engine configuration without recompiling.
//  */

// #include <string>
// #include <map>

// struct EngineOptions {
//     int hash_size_mb = 128;
//     int thread_count = 1;
//     bool own_book = true;
//     // Add other UCI options like "Ponder", "Contempt", etc.
// };

// // A global options object that can be accessed by the engine modules.
// // The UCI handler will be responsible for updating it.
// extern EngineOptions options;