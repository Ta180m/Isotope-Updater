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
#include <vector>

#include "AlertButtonView.hpp"
#include "FooterView.hpp"
#include "HeaderView.hpp"
#include "../ModalView.hpp"
#include "TextView.hpp"

namespace ku::views {
    class AlertView : public ku::ModalView {
        public:
            AlertView(std::string title, std::string message, std::vector<std::string> buttons);
            ~AlertView();

            void handleButton(u32 buttons, double dTime);
            void render(SDL_Rect rect, double dTime);
            int getSelectedOption();
            void reset();

        private:
            int _focusSelection;
            int _alertY;
            int _alertHeight;

            TextView * _titleTextView;
            TextView * _messageTextView;
            std::vector<AlertButtonView *> _buttons;
    };
}
