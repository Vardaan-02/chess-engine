#pragma once

/**
 * @file uci.h
 * @brief Declares the main loop for the Universal Chess Interface (UCI).
 *
 * This module is the "mouth and ears" of the engine, responsible for
 * communicating with a GUI or command line.
 */

namespace uci {
    // The main entry point for the UCI protocol.
    // This function will start a loop to listen for and process commands.
    void loop();
}