/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIDialogVideoScan.h"
#include "guilib/GUIProgressControl.h"
#include "GUIUserMessages.h"
#include "Util.h"
#include "guilib/GUIWindowManager.h"
#include "settings/GUISettings.h"
#include "ApplicationMessenger.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "URL.h"

#define CONTROL_LABELSTATUS       401
#define CONTROL_LABELDIRECTORY    402
#define CONTROL_PROGRESS          403
#define CONTROL_CURRENT_PROGRESS  404
#define CONTROL_LABELTITLE        405

using namespace VIDEO;

CGUIDialogVideoScan::CGUIDialogVideoScan(void)
: CGUIDialog(WINDOW_DIALOG_VIDEO_SCAN, "DialogVideoScan.xml")
{
  m_loadType = KEEP_IN_MEMORY;
}

CGUIDialogVideoScan::~CGUIDialogVideoScan(void)
{
}

bool CGUIDialogVideoScan::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_WINDOW_INIT:
    {
      CGUIDialog::OnMessage(message);

      m_strCurrentDir.Empty();
      m_strTitle.Empty();

      m_fPercentDone=-1.0f;
      m_fCurrentPercentDone=-1.0f;

      UpdateState();
      return true;
    }
    break;
  }

  return CGUIDialog::OnMessage(message);
}

void CGUIDialogVideoScan::FrameMove()
{
  if (m_active)
    UpdateState();

  CGUIDialog::FrameMove();
}

void CGUIDialogVideoScan::OnDirectoryChanged(const CStdString& strDirectory)
{
  CSingleLock lock (m_critical);

  m_strCurrentDir = strDirectory;
}

void CGUIDialogVideoScan::OnStateChanged(SCAN_STATE state)
{
  CSingleLock lock (m_critical);

  m_ScanState = state;
}

void CGUIDialogVideoScan::OnSetProgress(int currentItem, int itemCount)
{
  CSingleLock lock (m_critical);

  m_fPercentDone=(float)((currentItem*100)/itemCount);
  if (m_fPercentDone>100.0F) m_fPercentDone=100.0F;
}

void CGUIDialogVideoScan::OnSetCurrentProgress(int currentItem, int itemCount)
{
  CSingleLock lock (m_critical);

  m_fCurrentPercentDone=(float)((currentItem*100)/itemCount);
  if (m_fCurrentPercentDone>100.0F) m_fCurrentPercentDone=100.0F;
}

void CGUIDialogVideoScan::OnSetTitle(const CStdString& strTitle)
{
  CSingleLock lock (m_critical);

  m_strTitle = strTitle;
}

void CGUIDialogVideoScan::ShowScan()
{
  m_ScanState = PREPARING;
  Show();
}

void CGUIDialogVideoScan::OnDirectoryScanned(const CStdString& strDirectory)
{
  CGUIMessage msg(GUI_MSG_DIRECTORY_SCANNED, 0, 0, 0);
  msg.SetStringParam(strDirectory);
  g_windowManager.SendThreadMessage(msg);
}

void CGUIDialogVideoScan::OnFinished()
{
  // clear cache
  CUtil::DeleteVideoDatabaseDirectoryCache();

  // send message
  CGUIMessage msg(GUI_MSG_SCAN_FINISHED, 0, 0, 0);
  g_windowManager.SendThreadMessage(msg);

  // be sure to restore the settings
  CLog::Log(LOGINFO,"Video scan was stopped or finished ... restoring FindRemoteThumbs");

  if (!g_guiSettings.GetBool("videolibrary.backgroundupdate"))
  {
    CApplicationMessenger::Get().Close(this,false,false);
  }
}

void CGUIDialogVideoScan::UpdateState()
{
  CSingleLock lock (m_critical);

  SET_CONTROL_LABEL(CONTROL_LABELSTATUS, GetStateString());

  if (m_ScanState == FETCHING_MOVIE_INFO || m_ScanState == FETCHING_MUSICVIDEO_INFO || m_ScanState == FETCHING_TVSHOW_INFO || m_ScanState == CLEANING_UP_DATABASE)
  {
    CURL url(m_strCurrentDir);
    CStdString strStrippedPath = url.GetWithoutUserDetails();
    CURL::Decode(strStrippedPath);

    SET_CONTROL_LABEL(CONTROL_LABELDIRECTORY, strStrippedPath);
    SET_CONTROL_LABEL(CONTROL_LABELTITLE, m_strTitle);

    if (m_fCurrentPercentDone>-1.0f)
    {
      SET_CONTROL_VISIBLE(CONTROL_CURRENT_PROGRESS);
      CGUIProgressControl* pProgressCtrl=(CGUIProgressControl*)GetControl(CONTROL_CURRENT_PROGRESS);
      if (pProgressCtrl) pProgressCtrl->SetPercentage(m_fCurrentPercentDone);
    }
    else
      SET_CONTROL_HIDDEN(CONTROL_CURRENT_PROGRESS);

    if (m_fPercentDone>-1.0f)
    {
      SET_CONTROL_VISIBLE(CONTROL_PROGRESS);
      CGUIProgressControl* pProgressCtrl=(CGUIProgressControl*)GetControl(CONTROL_PROGRESS);
      if (pProgressCtrl) pProgressCtrl->SetPercentage(m_fPercentDone);
    }
    else
      SET_CONTROL_HIDDEN(CONTROL_PROGRESS);
  }
  else
  {
    SET_CONTROL_LABEL(CONTROL_LABELDIRECTORY, "");
    SET_CONTROL_LABEL(CONTROL_LABELTITLE, "");
    SET_CONTROL_HIDDEN(CONTROL_PROGRESS);
    SET_CONTROL_HIDDEN(CONTROL_CURRENT_PROGRESS);
  }
}

int CGUIDialogVideoScan::GetStateString()
{
  if (m_ScanState == PREPARING)
    return 314;
  else if (m_ScanState == REMOVING_OLD)
    return 701;
  else if (m_ScanState == CLEANING_UP_DATABASE)
    return 700;
  else if (m_ScanState == FETCHING_MOVIE_INFO)
    return 20374;
  else if (m_ScanState == FETCHING_MUSICVIDEO_INFO)
    return 20408;
  else if (m_ScanState == FETCHING_TVSHOW_INFO)
    return 20409;
  else if (m_ScanState == COMPRESSING_DATABASE)
    return 331;
  else if (m_ScanState == WRITING_CHANGES)
    return 328;

  return -1;
}