/*
 * EntryPoint.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"

#include "LobbyServer.h"
#include "HttpServer.h"
#include "LobbyHttpApi.h"

#include "../lib/CConsoleHandler.h"
#include "../lib/logging/CBasicLogConfigurator.h"
#include "../lib/filesystem/CFilesystemLoader.h"
#include "../lib/filesystem/Filesystem.h"
#include "../lib/VCMIDirs.h"

static const int LISTENING_PORT = 3031;
static const int HTTP_API_PORT = 3032;
static const bool HTTP_API_LOCALHOST_ONLY = true;

int main(int argc, const char * argv[])
{
	std::optional<boost::filesystem::path> dedicatedServerExecutable;
	for(int index = 1; index < argc; ++index)
	{
		const std::string argument = argv[index];
		if(argument == "--enable-dedicated-servers")
			dedicatedServerExecutable = boost::filesystem::absolute(argv[0]).parent_path() / "vcmiserver";
		else if(argument.starts_with("--dedicated-server="))
			dedicatedServerExecutable = argument.substr(std::string("--dedicated-server=").size());
	}

	CResourceHandler::initialize();
	CResourceHandler::load("config/filesystem.json"); // FIXME: we actually need only config directory for schemas, can be reduced

#ifndef VCMI_IOS
	CConsoleHandler console;
#endif
	CBasicLogConfigurator logConfigurator(VCMIDirs::get().userLogsPath() / "VCMI_Lobby_log.txt", &console);
	logConfigurator.configureDefault();

	auto databasePath = VCMIDirs::get().userDataPath() / "vcmiLobby.db";
	logGlobal->info("Opening database %s", databasePath.string());

	if(dedicatedServerExecutable)
	{
		if(!boost::filesystem::exists(*dedicatedServerExecutable))
		{
			logGlobal->error("Dedicated server executable does not exist: %s", dedicatedServerExecutable->string());
			return 1;
		}
		logGlobal->info("Dedicated server allocation enabled using %s", dedicatedServerExecutable->string());
	}

	LobbyServer server(databasePath, dedicatedServerExecutable);
	logGlobal->info("Starting server on port %d", LISTENING_PORT);

	try
	{
		server.start(LISTENING_PORT);
	}
	catch (const boost::system::system_error & e)
	{
		logGlobal->error("Failed to start server! Another server already uses the same port? Reason: '%s'", e.what());
		return 1;
	}

	// Start HTTP API Server
	LobbyHttpApi lobbyApi(*server.getDatabase());
	HttpServer httpServer(server.getNetworkContext(), lobbyApi, HTTP_API_PORT, HTTP_API_LOCALHOST_ONLY);
	try
	{
		httpServer.start();
	}
	catch (const std::exception & e)
	{
		logGlobal->error("Failed to start HTTP API server! Reason: '%s'", e.what());
		return 1;
	}

	server.run();

	httpServer.stop();

	return 0;
}
