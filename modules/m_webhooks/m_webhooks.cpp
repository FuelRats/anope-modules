/*
 * Allows events in Anope to trigger calls to remote webhook endpoints.
 *
 * (C) 2017 - Timothy Gunter <gunter.tim@gmail.com>
 *
 * Please refer to the GPL License in use by Anope at:
 * https://github.com/anope/anope/blob/master/docs/COPYING
 *
 */

/* RequiredLibraries: curl */

/*
 * Configuration to put into your modules config:

module
{
	name = "m_webhooks"
	webhook = "https://domain/path"

	ns_drop = "no"
	ns_register = "no"
	ns_group = "no"

	timeout = "5"
	connect_timeout = "5"
}

 */

#include "module.h"
#include <curl/curl.h>
#include <iomanip>
#include <sstream>

class ModuleWebhooks : public Module
{
	Anope::string webhook_url;
	long timeout, connect_timeout;
	bool ns_drop, ns_register, ns_group;

public:

	ModuleWebhooks(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, EXTRA | VENDOR) {
		curl_global_init(CURL_GLOBAL_ALL);
	}

	~ModuleWebhooks() {

	}

	void OnReload(Configuration::Conf *conf) anope_override {
		Log(LOG_DEBUG) << "WEBHOOK config reload";
		Configuration::Block *block = Config->GetModule(this);

		this->webhook_url = block->Get<const Anope::string>("webhook", "none");
		Log(LOG_DEBUG) << "WEBHOOK config webhook_url: " << this->webhook_url;

		this->ns_drop = block->Get<bool>("ns_drop", "no");
		this->ns_register = block->Get<bool>("ns_register", "no");
		this->ns_group = block->Get<bool>("ns_group", "no");
		this->timeout = block->Get<long>("timeout", "5");
		this->connect_timeout = block->Get<long>("connect_timeout", "5");
	}

	void OnNickDrop(CommandSource &source, NickAlias *na) {
		if (ns_drop && na)
			PostCall("ns_drop", na->nick.c_str());
	}

	void OnNickRegister(User *user, NickAlias *na, const Anope::string &pass) {
		if (ns_register && na)
			PostCall("ns_register", na->nick.c_str());
	}

	void OnNickGroup(User *user, NickAlias *na) {
		if (ns_group && na)
			PostCall("ns_group", na->nick.c_str());
	}

private:
	void PostCall(const std::string &event, const char *user) {
		Log(LOG_NORMAL) << "WEBHOOK request [" << event << "] sent to " << this->webhook_url;

		CURL *curl = curl_easy_init();
		if (curl) {

			Anope::string ua = "Anope/" + Anope::Version();

			curl_easy_setopt(curl, CURLOPT_URL, webhook_url.c_str());
			curl_easy_setopt(curl, CURLOPT_USERAGENT, ua.c_str());

			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
			curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 0L);

			// Build fields
			Anope::string data = "event="+event+"&user="+user;
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

			CURLcode res = curl_easy_perform(curl);

			if (res > 0) {
				Log(LOG_NORMAL) << "WEBHOOK error [" << res << "] " << curl_easy_strerror(res);
			} else {
				// Check response
				long response_code;
				double d_elapsed;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
				curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &d_elapsed);

				int statuscode = (int) response_code;
				std::stringstream ss;
				ss << std::fixed << std::setprecision(2) << d_elapsed;
				Log(LOG_NORMAL) << "WEBHOOK response [" << statuscode << "] in " << ss.str() << "s";
			}

			curl_easy_cleanup(curl);
		}
	}
};

MODULE_INIT(ModuleWebhooks)