// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "math.h"
#include "defines.h" // IWYU pragma: keep
#include "iwObservate.h"
#include "Loader.h"
#include "driver/src/MouseCoords.h"
#include "drivers/VideoDriverWrapper.h"
#include "world/GameWorldView.h"
#include "world/GameWorldViewer.h"
#include "world/GameWorldBase.h"
#include "Settings.h"
#include "controls/ctrlButton.h"
#include "gameTypes/RoadBuildState.h"
#include "ogl/glArchivItem_Bitmap.h"
#include "libutil/src/Log.h"
#include <boost/foreach.hpp>

iwObservate::iwObservate(GameWorldView& gwv, const MapPoint selectedPt):
    IngameWindow(gwv.GetWorld().CreateGUIID(selectedPt), 0xFFFE, 0xFFFE, 300, 250, _("Observation window"), NULL),
    parentView(gwv),
    view(new GameWorldView(gwv.GetViewer(), Point<int>(GetX() + 10, GetY() + 15), 300 - 20, 250 - 20)),
    selectedPt(selectedPt), lastWindowPos(Point<unsigned short>::Invalid()), isScrolling(false), zoomLvl(0),
    followMovableId(GameObject::INVALID_ID)
{
    view->MoveToMapPt(selectedPt);
    SetCloseOnRightClick(false);

    // Lupe: 36
    AddImageButton(1, GetWidth() / 2 - 36 * 2, GetHeight() - 50, 36, 36, TC_GREY, LOADER.GetImageN("io", 36), _("Zoom"));
    // Kamera (Folgen): 43
    AddImageButton(2, GetWidth() / 2 - 36, GetHeight() - 50, 36, 36, TC_GREY, LOADER.GetImageN("io", 43), _("Follow object"));
    // Zum Ort
    AddImageButton(3, GetWidth() / 2, GetHeight() - 50, 36, 36, TC_GREY, LOADER.GetImageN("io", 107), _("Go to place"));
    // Fenster vergroessern/verkleinern
    AddImageButton(4, GetWidth() / 2 + 36, GetHeight() - 50, 36, 36, TC_GREY, LOADER.GetImageN("io", 109), _("Resize window"));
}

void iwObservate::Msg_ButtonClick(const unsigned int ctrl_id)
{
    switch (ctrl_id)
    {
        case 1:
            if(++zoomLvl > 4)
                zoomLvl = 0;
            if(zoomLvl == 0)
                view->SetZoomFactor(1.f);
            else if(zoomLvl == 1)
                view->SetZoomFactor(1.3f);
            else if(zoomLvl == 2)
                view->SetZoomFactor(1.6f);
            else if(zoomLvl == 3)
                view->SetZoomFactor(1.9f);
            else
                view->SetZoomFactor(2.3f);
            break;
        case 2:
        {
            if (followMovableId != GameObject::INVALID_ID)
                followMovableId = GameObject::INVALID_ID;
            else
            {
                const DrawPoint centerDrawPt = DrawPoint(view->GetSize() / 2);

                double minDistance = std::numeric_limits<double>::max();

                for(int y = view->GetFirstPt().y; y <= view->GetLastPt().y; ++y)
                {
                    for(int x = view->GetFirstPt().x; x <= view->GetLastPt().x; ++x)
                    {
                        Point<int> curOffset;
                        const MapPoint curPt = view->GetViewer().GetTerrainRenderer().ConvertCoords(Point<int>(x, y), &curOffset);
                        DrawPoint curDrawPt = view->GetWorld().GetNodePos(curPt) - view->GetOffset() + curOffset;

                        if(view->GetViewer().GetVisibility(curPt) != VIS_VISIBLE)
                            continue;

                        const std::list<noBase*>& figures = view->GetWorld().GetFigures(curPt);

                        BOOST_FOREACH(const noBase* obj, figures)
                        {
                            const noMovable *movable = dynamic_cast<const noMovable*>(obj);
                            if(!movable)
                                continue;

                            DrawPoint objDrawPt = curDrawPt;

                            if (movable->IsMoving())
                                objDrawPt += movable->CalcWalkingRelative();

                            DrawPoint diffToCenter = objDrawPt - centerDrawPt;
                            double distance = sqrt(pow(diffToCenter.x, 2) + pow(diffToCenter.y, 2));

                            if (distance < minDistance)
                            {
                                followMovableId = movable->GetObjId();
                                minDistance = distance;
                            }
                        }
                    }
                }
            }

            break;
        }
        case 3:
            parentView.MoveToMapPt( MapPoint(view->GetLastPt() - (view->GetLastPt() - view->GetFirstPt()) / 2) );
            break;
        case 4:
            int diff = width_;

            if (width_ == 260)
            {
                SetWidth(300);
                SetIwHeight(250);
            }
            else if (width_ == 300)
            {
                SetWidth(340);
                SetIwHeight(310);
                GetCtrl<ctrlImageButton>(4)->SetImage(LOADER.GetImageN("io", 108));
            }
            else
            {
                SetWidth(260);
                SetIwHeight(190);
                GetCtrl<ctrlImageButton>(4)->SetImage(LOADER.GetImageN("io", 109));
            }

            diff -= width_;
            diff /= 2;

            view->Resize(width_ - 20, height_ - 20);

            for (unsigned i = 1; i <= 4; ++i)
                GetCtrl<ctrlImageButton>(i)->Move(GetCtrl<ctrlImageButton>(i)->GetX(false) - diff, GetHeight() - 50);

            if (x_ + width_ >= VIDEODRIVER.GetScreenWidth())
            {
                Move(VIDEODRIVER.GetScreenWidth() - width_ - 1, y_);
            }

            if (y_ + height_ >= VIDEODRIVER.GetScreenHeight())
            {
                Move(x_, VIDEODRIVER.GetScreenHeight() - height_ - 1);
            }
    }
}

bool iwObservate::Draw_()
{
    if ((x_ != lastWindowPos.x) || (y_ != lastWindowPos.y))
    {
        view->SetPos(Point<int>(GetX() + 10, GetY() + 15));
        lastWindowPos = Point<unsigned short>(x_, y_);
    }

    if (followMovableId != GameObject::INVALID_ID)
    {
        if(!MoveToFollowedObj())
            followMovableId = GameObject::INVALID_ID;
    }

    if (!IsMinimized())
    {
        RoadBuildState road;
        road.mode = RM_DISABLED;

        view->Draw(road, true, parentView.GetSelectedPt());
        // Draw indicator for center point
        if (followMovableId == GameObject::INVALID_ID)
            LOADER.GetMapImageN(23)->Draw(view->GetPos() + DrawPoint(view->GetSize() / 2));
    }

    return IngameWindow::Draw_();
}

bool iwObservate::MoveToFollowedObj()
{
    // First look at the center (figure is normally still there)
    const MapPoint centerPt = view->GetWorld().MakeMapPoint((view->GetLastPt() - view->GetFirstPt()) / 2);
    const std::vector<MapPoint> centerPts = view->GetWorld().GetPointsInRadiusWithCenter(centerPt, 1);
    BOOST_FOREACH(const MapPoint& curPt, centerPts)
    {
        if(MoveToFollowedObj(curPt))
            return true;
    }

    // Not at the center (normally due to lags) -> Check full area
    for(int y = view->GetFirstPt().y; y <= view->GetLastPt().y; ++y)
    {
        for(int x = view->GetFirstPt().x; x <= view->GetLastPt().x; ++x)
        {
            const MapPoint curPt = view->GetWorld().MakeMapPoint(Point<int>(x, y));
            if(MoveToFollowedObj(curPt))
                return true;
        }
    }
    return false;
}

bool iwObservate::MoveToFollowedObj(const MapPoint ptToCheck)
{
    if(view->GetViewer().GetVisibility(ptToCheck) != VIS_VISIBLE)
        return false;
    const std::list<noBase*>& curObjs = view->GetWorld().GetFigures(ptToCheck);
    BOOST_FOREACH(const noBase* obj, curObjs)
    {
        if(obj->GetObjId() == followMovableId)
        {
            const noMovable* followMovable = static_cast<const noMovable*>(obj);
            DrawPoint drawPt = view->GetWorld().GetNodePos(ptToCheck);

            if (followMovable->IsMoving())
                drawPt += followMovable->CalcWalkingRelative();

            view->MoveTo(drawPt - DrawPoint(view->GetSize()) / 2, true);
            return true;
        }
    }
    return false;
}

bool iwObservate::Msg_MouseMove(const MouseCoords& mc)
{
    if (isScrolling)
    {
        int acceleration = SETTINGS.global.smartCursor ? 2 : 3;

        if(SETTINGS.interface.revert_mouse)
            acceleration = -acceleration;

        view->MoveTo((DrawPoint(mc.x, mc.y) - scrollOrigin));
        VIDEODRIVER.SetMousePos(scrollOrigin);
    }

    return(false);
}

bool iwObservate::Msg_RightDown(const MouseCoords& mc)
{
    scrollOrigin.x = mc.x;
    scrollOrigin.y = mc.y;

    isScrolling = true;
    followMovableId = GameObject::INVALID_ID;

    return(false);
}

bool iwObservate::Msg_RightUp(const MouseCoords&  /*mc*/)
{
    isScrolling = false;

    return(false);
}
