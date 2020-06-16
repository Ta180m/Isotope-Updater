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

#pragma once

#include <string>
#include <Swurl.hpp>

#include "../ModalView.hpp"
#include "../Scene.hpp"
#include "../views/AlertView.hpp"
#include "../views/FooterView.hpp"
#include "../views/HeaderView.hpp"
#include "../views/StatusView.hpp"
#include "../views/UpdateView.hpp"

namespace dsu::scenes {
    class PackageDownloadScene : public dsu::Scene {
        public:
            PackageDownloadScene();
            ~PackageDownloadScene();

            void handleButton(u32 buttons, double dTime);
            void render(SDL_Rect rect, double dTime);

        private:
            dsu::views::HeaderView * _headerView = NULL;
            dsu::views::UpdateView * _updateView = NULL;
            dsu::views::StatusView * _statusView = NULL;
            dsu::views::FooterView * _footerView = NULL;
            dsu::views::AlertView * _restartAlertView = NULL;

            std::string _IsotopeVersion = "";

            swurl::WebRequest * _IsotopeUrlRequest = NULL;
            swurl::WebRequest * _IsotopeRequest = NULL;

            void _copyToIram(uintptr_t iram_addr, void *buf, size_t size);
            void _clearIram();

            void _showStatus(std::string text, std::string subtext, bool wasSuccessful);
            void _onAlertViewDismiss(dsu::ModalView * view, bool success);
            std::string _getVersionNumber(std::string version);

            void _onProgressUpdate(swurl::WebRequest * request, double progress);
            void _onCompleted(swurl::WebRequest * request);
            void _onError(swurl::WebRequest * request, std::string error);
    };
}
