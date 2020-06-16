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

#include "ImageView.hpp"
#include "ProgressBarView.hpp"
#include "TextView.hpp"
#include "../View.hpp"

namespace ku::views {
    class UpdateView : public ku::View {
        public:
            UpdateView(std::string text);
            ~UpdateView();

            void render(SDL_Rect rect, double dTime);
            void setProgress(double progress);
            void setText(std::string text);
            void setProgressBarHidden(bool hidden);

        private:
            ImageView * _downloadImageView;
            ProgressBarView * _progressBarView;
            TextView * _statusTextView;
    };
}
