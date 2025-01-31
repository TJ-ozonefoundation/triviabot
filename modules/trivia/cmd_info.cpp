/************************************************************************************
 * 
 * TriviaBot, The trivia bot for discord based on Fruitloopy Trivia for ChatSpike IRC
 *
 * Copyright 2004 Craig Edwards <support@brainbox.cc>
 *
 * Core based on Sporks, the Learning Discord Bot, Craig Edwards (c) 2019.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/

#include <dpp/dpp.h>
#include <fmt/format.h>
#include <dpp/nlohmann/json.hpp>
#include <sporks/modules.h>
#include <sporks/regex.h>
#include <string>
#include <cstdint>
#include <fstream>
#include <streambuf>
#include <sporks/stringops.h>
#include <sporks/statusfield.h>
#include <sporks/database.h>
#include "trivia.h"
#include "webrequest.h"
#include "commands.h"

using json = nlohmann::json;

command_info_t::command_info_t(class TriviaModule* _creator, const std::string &_base_command, bool adm, const std::string& descr, std::vector<dpp::command_option> options) : command_t(_creator, _base_command, adm, descr, options) { }

void command_info_t::call(const in_cmd &cmd, std::stringstream &tokens, guild_settings_t &settings, const std::string &username, bool is_moderator, dpp::channel* c, dpp::user* user)
{
	std::stringstream s;
	dpp::utility::uptime ut = creator->GetBot()->core->uptime();

	int64_t servers = creator->GetGuildTotal();
	int64_t users = creator->GetMemberTotal();

	char startstr[256];
	tm _tm;
	gmtime_r(&creator->startup, &_tm);
	strftime(startstr, 255, "%x, %I:%M%p", &_tm);

	uint64_t shard = (cmd.guild_id >> 22) % from_string<uint32_t>(Bot::GetConfig("shardcount"), std::dec);

	const statusfield statusfields[] = {
		statusfield(_("ACTIVEGAMES", settings), Comma(creator->GetActiveGames())),
		statusfield(_("TOTALSERVERS", settings), Comma(servers)),
		statusfield(_("CONNSINCE", settings), startstr),
		statusfield(_("ONLINEUSERS", settings), Comma(users)),
		statusfield(_("UPTIME", settings), ut.to_string()),
		statusfield(_("CLUSTER", settings), Comma(creator->GetBot()->GetClusterID()) + "/" + Comma(creator->GetBot()->GetMaxClusters())),
		statusfield(_("SHARDS", settings), Comma(shard) + "/" + Bot::GetConfig("shardcount")),
		statusfield(_("MEMBERINTENT", settings), _((creator->GetBot()->HasMemberIntents() ? "TICKYES" : "CROSSNO"), settings)),
		statusfield(_("TESTMODE", settings), _((creator->GetBot()->IsTestMode() ? "TICKYES" : "CROSSNO"), settings)),
		statusfield(_("DEVMODE", settings), _((creator->GetBot()->IsDevMode() ? "TICKYES" : "CROSSNO"), settings)),
		statusfield(_("MYPREFIX", settings), "`" + creator->escape_json(settings.prefix) + "`"),
		statusfield(_("BOTVER", settings), std::string(creator->GetVersion())),
		statusfield(_("LIBVER", settings), "<:DPP1:847152435399360583><:DPP2:847152435343523881> [" + std::string(DPP_VERSION_TEXT) + "](https://dpp.dev/)"),
		statusfield("", "")
	};

	s << "{\"title\":\"" << creator->GetBot()->user.username << " " << _("INFO", settings);
	s << "\",\"color\":" << settings.embedcolour << ",\"url\":\"https:\\/\\/triviabot.co.uk\\/\\/\",";
	s << "\"footer\":{\"link\":\"https:\\/\\/triviabot.co.uk\\/\",\"text\":\"" << _("POWERED_BY", settings) << "\",\"icon_url\":\"https:\\/\\/triviabot.co.uk\\/images\\/triviabot_tl_icon.png\"},\"fields\":[";
	for (int i = 0; statusfields[i].name != ""; ++i) {
		s << "{\"name\":\"" <<  statusfields[i].name << "\",\"value\":\"" << statusfields[i].value << "\", \"inline\": true}";
		if (statusfields[i + 1].name != "") {
			s << ",";
		}
	}
	s << "],\"description\":\"" << (settings.premium ? _("YAYPREMIUM", settings) : "") << "\"}";

	creator->ProcessEmbed(cmd.interaction_token, cmd.command_id, settings, s.str(), cmd.channel_id);
	creator->CacheUser(cmd.author_id, cmd.user, cmd.member, cmd.channel_id);
}

