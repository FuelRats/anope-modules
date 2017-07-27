/*
 * Allows events in Anope to trigger calls to remote webhook endpoints.
 *
 * (C) 2017 - Timothy Gunter <gunter.tim@gmail.com>
 *
 * Please refer to the GPL License in use by Anope at:
 * https://github.com/anope/anope/blob/master/docs/COPYING
 *
 */

/*
 * Configuration to put into your modules config:

module
{
	name = "m_webhooks"
    url = "https://domain/path"

    ns_drop = "yes"
    ns_register = "yes"
    ns_confirm = "yes"
}

 */

#include "module.h"
#include <curl/curl.h>

class ModuleWebhooks : public Module
{

	Anope::string webhook_url;
	bool ns_drop, ns_register, ns_confirm;

public:
	ModuleWebhooks(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, EXTRA | VENDOR) {
		curl_global_init(CURL_GLOBAL_ALL);
	}

	~ModuleWebhooks() {

	}

	void OnReload(Configuration::Conf *conf) anope_override {
		webhook_url = Config->GetModule(this)->Get<const Anope::string>("url", "");
		ns_drop = Config->GetModule(this)->Get<bool>("ns_drop", "no");
		ns_register = Config->GetModule(this)->Get<bool>("ns_register", "no");
		ns_confirm = Config->GetModule(this)->Get<bool>("ns_confirm", "no");
	}

	void OnNickDrop(CommandSource &source, NickAlias *na) {
		if (ns_drop)
			this->PostCall("ns_drop", this->webhook_url, source.GetUser());
	}

	void OnNickRegister(User *user, NickAlias *na, const Anope::string &pass) {
		if (ns_register)
			this->PostCall("ns_register", this->webhook_url, user);
	}

	void OnNickConfirm(User *user, NickCore *nc) {
		if (ns_confirm)
			this->PostCall("ns_confirm", this->webhook_url, user);
	}

private:
	void PostCall(const std::string &event, Anope::string &url, User *user) {
		Log(LOG_DEBUG) << "m_webhooks: posting to " << url << " for '" << event << "'";

		CURL *curl = curl_easy_init();
		if (curl) {

			Anope::string ua = "Anope/" + Anope::Version();

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, ua.c_str());
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
			curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 0L);

			// Build fields
			Anope::string data = "event="+event+"&user="+user->nick;
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

			curl_easy_perform(curl);
			curl_easy_cleanup(curl);
		}
	}

	static size_t WriteFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
		data->append((char*) ptr, size * nmemb);
		return size * nmemb;
	}
};

MODULE_INIT(ModuleWebhooks)