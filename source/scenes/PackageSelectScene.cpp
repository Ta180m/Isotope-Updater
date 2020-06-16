// Isotope Updater
// Copyright (C) 2020 Nichole Mattera
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include <jansson.h>

#include "PackageSelectScene.hpp"

#include "../ConfigManager.hpp"
#include "../SceneDirector.hpp"
#include "../FileManager.hpp"

using namespace dsu;
using namespace dsu::models;
using namespace dsu::views;
using namespace std;
using namespace std::placeholders;
using namespace swurl;

namespace dsu::scenes
{
    PackageSelectScene::PackageSelectScene()
    {
        SessionManager::onProgressChanged = bind(&PackageSelectScene::_onProgressUpdate, this, _1, _2);
        SessionManager::onCompleted = bind(&PackageSelectScene::_onCompleted, this, _1);
        SessionManager::onError = bind(&PackageSelectScene::_onError, this, _1, _2);

        _headerView = new HeaderView("Isotope Updater", true);
        _headerView->frame = {0, 0, 1280, 88};

        _updateView = new UpdateView("Checking for updates to Isotope...");
        _updateView->frame.x = 0;
        _updateView->frame.y = 200;

        _statusView = new StatusView("", "");
        _statusView->frame.x = 0;
        _statusView->frame.y = 323;

        _installRowView = new ListRowView("Install Latest Isotope", "", SUBTITLE);
        _installRowView->frame.x = 215;
        _installRowView->frame.y = 137;
        _installRowView->isLast = true;
        _installRowView->hasFocus = true;

        _footerView = new FooterView();
        _footerView->frame = {0, 647, 1280, 73};

        vector<string> buttons;
        buttons.push_back("Yes");
        buttons.push_back("No");
        _ignoreConfigsAlertView = new AlertView("Ignore Config Files?", "Would you like for Isotope Updater to ignore config\nfiles? This will prevent Isotope Updater from overwriting\nall config files except for Hekate's main config file, and save the\nignored files for next time as well.", buttons);
        _ignoreConfigsAlertView->onDismiss = bind(&PackageSelectScene::_onAlertViewDismiss, this, _1, _2);

        addSubView(_headerView);
        addSubView(_updateView);
        addSubView(_statusView);
        addSubView(_installRowView);
        addSubView(_footerView);

        _showUpdateView();
    }

    PackageSelectScene::~PackageSelectScene()
    {
        if (_headerView != NULL)
            delete _headerView;

        if (_updateView != NULL)
            delete _updateView;

        if (_statusView != NULL)
            delete _statusView;

        if (_installRowView != NULL)
            delete _installRowView;

        if (_footerView != NULL)
            delete _footerView;

        if (_IsotopeVersionRequest != NULL)
            delete _IsotopeVersionRequest;
    }

    void PackageSelectScene::handleButton(u32 buttons, double dTime)
    {
        if (!_statusView->hidden && buttons & KEY_A)
        {
            SceneDirector::exitApp = true;
        }
        else if (_updateView->hidden && _statusView->hidden)
        {
            if (buttons & KEY_A)
            {
                SceneDirector::currentScene = SCENE_PACKAGE_DOWNLOAD;
            }

            if (buttons & KEY_B)
            {
                SceneDirector::exitApp = true;
            }
        }
    }

    void PackageSelectScene::render(SDL_Rect rect, double dTime)
    {
        if (_IsotopeVersionRequest == NULL)
        {
            _IsotopeVersionRequest = new WebRequest("https://api.github.com/repos/Ta180m/Isotope/releases");
            SceneDirector::currentSceneDirector->render();
            SessionManager::makeRequest(_IsotopeVersionRequest);
        }

        Scene::render(rect, dTime);
    }

    void PackageSelectScene::_showUpdateView()
    {
        _updateView->setProgress(0);
        _updateView->hidden = false;
        _statusView->hidden = true;
        _installRowView->hidden = true;

        for (auto const &action : _footerView->actions)
        {
            delete action;
        }
        _footerView->actions.clear();
    }

    void PackageSelectScene::_showPackageSelectViews(std::string IsotopeVersion)
    {
        // if (!ConfigManager::getReceivedIgnoreConfigWarning())
        // {
        _ignoreConfigsAlertView->show();
        // }
        
        _updateView->hidden = true;
        _statusView->hidden = true;

        _installRowView->hidden = false;
        _installRowView->hasFocus = true;

        string version = ConfigManager::getCurrentVersion();
        if (version.compare(IsotopeVersion) == 0)
        {
            _installRowView->setPrimaryText("Reinstall Isotope");
        }
        else
        {
            _installRowView->setPrimaryText("Install Latest Isotope");
        }

        if (version == "" || version.compare(IsotopeVersion) == 0)
        {
            _installRowView->setSecondaryText("Latest Version is " + IsotopeVersion);
        }
        else
        {
            _installRowView->setSecondaryText("You currently have version " + version + " installed, and the latest version is " + IsotopeVersion + ".");
        }

        for (auto const &action : _footerView->actions)
        {
            delete action;
        }
        _footerView->actions.clear();
        _footerView->actions.push_back(new Action(A_BUTTON, "OK"));
        _footerView->actions.push_back(new Action(B_BUTTON, "Quit"));
    }

    void PackageSelectScene::_showStatusView(string text, string subtext)
    {
        _statusView->setText(text);
        _statusView->setSubtext(subtext);

        _updateView->hidden = true;
        _statusView->hidden = false;
        _installRowView->hidden = true;

        for (auto const &action : _footerView->actions)
        {
            delete action;
        }
        _footerView->actions.clear();
        _footerView->actions.push_back(new Action(A_BUTTON, "Quit"));
    }

    // Alert View Callback Method

    void PackageSelectScene::_onAlertViewDismiss(ModalView *view, bool success)
    {
        if (success && _ignoreConfigsAlertView->getSelectedOption() == 0)
        {
            vector<string> files = FileManager::scanDirectoryRecursive("sdmc:/config");
            vector<string> filesToIgnore = ConfigManager::getFilesToIgnore();
            files.push_back("sdmc:/atmosphere/config/BCT.ini");
            files.push_back("sdmc:/atmosphere/config/override_config.ini");
            files.push_back("sdmc:/atmosphere/config/system_settings.ini");
            files.push_back("sdmc:/bootloader/patches.ini");
            files.push_back("sdmc:/bootloader/hekate_ipl.ini");
            files.push_back("sdmc:/switch/Isotope-Toolbox/config.json");
            files.push_back("sdmc:/switch/Isotope-Updater/internal.db");
            files.push_back("sdmc:/switch/Isotope-Updater/settings.cfg");

            if(filesToIgnore.empty())
            {
                ConfigManager::setFilesToIgnore(files);
            } else {
                for (auto i : filesToIgnore)
                {
                    files.erase(remove(files.begin(), files.end(), i), files.end());
                }
            }
            ConfigManager::setIgnoreConfigFiles(true);
        } else {
            ConfigManager::setIgnoreConfigFiles(false);
        }
    }

    // Swurl Callback Methods

    void PackageSelectScene::_onProgressUpdate(WebRequest *request, double progress)
    {
        _updateView->setProgress(progress);
        SceneDirector::currentSceneDirector->render();
    }

    void PackageSelectScene::_onCompleted(WebRequest *request)
    {
        json_t *root = json_loads(request->response.rawResponseBody.c_str(), 0, NULL);
        if (!root || !json_is_array(root) || json_array_size(root) < 1)
        {
            _showStatusView("Unable to parse response from GitHub API.", "Please restart the app to try again.[1]");
            return;
        }

        json_t *release = json_array_get(root, 0);
        if (!release || !json_is_object(release))
        {
            _showStatusView("Unable to parse response from GitHub API.", "Please restart the app to try again.[2]");
            return;
        }

        json_t *tagName = json_object_get(release, "tag_name");
        if (!tagName || !json_is_string(tagName))
        {
            _showStatusView("Unable to parse response from GitHub API.", "Please restart the app to try again.[3]");
            return;
        }

        _showPackageSelectViews(json_string_value(tagName));

        json_decref(root);

        SessionManager::onProgressChanged = NULL;
        SessionManager::onCompleted = NULL;
        SessionManager::onError = NULL;
    }

    void PackageSelectScene::_onError(WebRequest *request, string error)
    {
        _showStatusView(error, "Please restart the app to try again.");
    }
} // namespace dsu::scenes
