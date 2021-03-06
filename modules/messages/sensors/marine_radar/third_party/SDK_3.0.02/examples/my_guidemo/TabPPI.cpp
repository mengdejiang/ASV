//-----------------------------------------------------------------------------
// Copyright (C) 2007-2011 Navico
// Confidential and proprietary.  All rights reserved.
//-----------------------------------------------------------------------------

#include "TabPPI.h"

static float CompassToDegrees(uint16_t compass) {
  return static_cast<float>(compass) * 360.0f / 4096.0f;
}

static float ToDegrees(float degrees) {
  float d = 360.0f + 90.0f - degrees;
  while (d < 0.0f) d += 360.0f;
  while (d > 360.0f) d -= 360.0f;
  return d;
}

//-----------------------------------------------------------------------------
//  tQPPIFrame Implementation
//-----------------------------------------------------------------------------
void tQPPIFrame::addActions(QMenu& menu) {
  static const int trailTimes[] = {0, 15, 30, 1 * 60, 2 * 60, 5 * 60, -1};
  QMenu* pTrailsMenu = menu.addMenu("PPI Trails Time");

  int time = 0;
  for (unsigned i = 0; time >= 0; ++i) {
    time = trailTimes[i];

    QString text;
    if (time < 0)
      text = "Infinite";
    else if (time < 60)
      text = QString("%0 seconds").arg(time);
    else if (time == 60)
      text = "1 minute";
    else
      text = QString("%0 minutes").arg(time / 60);

    QAction* pAction = pTrailsMenu->addAction(text);
    pAction->setData(time);
    pAction->setCheckable(true);
    if (time == m_TrailsTime) pAction->setChecked(true);
  }
}

//----------------------------------------------------------
void tQPPIFrame::performAction(QAction& action) {
  m_TrailsTime = action.data().toInt();
  emit ChangeTrailsTime(m_TrailsTime);
}

//----------------------------------------------------------
void tQPPIFrame::convertXYtoSD(int x, int y, double& s, double& d) {
  double xx = getImage()->width() * (x - width() / 2.0) / width();
  double yy = getImage()->height() * (height() / 2 - y) / height();
  s = std::sqrt(xx * xx + yy * yy);
  d = std::atan2(xx, yy);
  if (d < 0.0) d += 2.0 * M_PI;
  d = d * 180.0 / M_PI;
}

//----------------------------------------------------------
void tQPPIFrame::convertSDtoXY(double s, double d, int& x, int& y) {
  double radians = M_PI * d / 180.0;
  x = static_cast<int>(width() / 2 +
                       (width() * s * std::sin(radians)) / getImage()->width() +
                       0.5);
  y = static_cast<int>(
      height() / 2 - (height() * s * std::cos(radians)) / getImage()->height() +
      0.5);
}

void tQPPIFrame::DrawOverlay(QPainter& painter, const tOverlay* pOverlay) {
  int width = this->width();
  int height = this->height();
  double dist1 = static_cast<double>(pOverlay->startRange_m) / m_FullRange_m;
  double dist2 = static_cast<double>(pOverlay->endRange_m) / m_FullRange_m;
  float deg1 = ToDegrees(pOverlay->startBearing_deg + m_Bearing_deg);
  float deg2 = ToDegrees(pOverlay->endBearing_deg + m_Bearing_deg);
  float delta = deg2 - deg1;
  QRectF inner(-dist1 * width, -dist1 * height, 2 * dist1 * width,
               2 * dist1 * height);
  QRectF outer(-dist2 * width, -dist2 * height, 2 * dist2 * width,
               2 * dist2 * height);

  painter.setPen(pOverlay->lineColour);
  painter.setBrush(QBrush(pOverlay->fillColour));

  QPainterPath path;

  if (pOverlay->startRange_m == 0u) {
    path.arcMoveTo(outer, deg1);
    QPointF pos1 = path.currentPosition();
    path.arcTo(outer, deg1, delta);
    path.lineTo(0, 0);
    path.lineTo(pos1);
  } else {
    path.arcMoveTo(inner, deg1);
    QPointF pos1 = path.currentPosition();
    path.arcMoveTo(outer, deg2);
    QPointF pos2 = path.currentPosition();

    path.arcTo(outer, deg2, -delta);
    path.lineTo(pos1);
    path.arcTo(inner, deg1, delta);
    path.lineTo(pos2);
  }
  path.closeSubpath();

  painter.drawPath(path);
}

//-----------------------------------------------------------------------------
//  tTabPPI Implementation
//-----------------------------------------------------------------------------
tTabPPI::tTabPPI(Ui::GUIDemoClass& myUI, tTargetLocation* pTargets,
                 unsigned maxTargets, QObject* pParent,
                 tOverlayManager& overlayManager)
    : QObject(pParent),
      m_pFrame(new tQPPIFrame(pTargets, maxTargets, myUI.tabPPI, nullptr,
                              overlayManager)),
      m_pController(new Navico::Image::tPPIController()),
      m_pImage(nullptr),
      ui(myUI) {
  ui.verticalLayout_tabPPI->addWidget(m_pFrame);
  Connect(true, &m_Timer, SIGNAL(timeout()), this, SLOT(Timer_timeout()));
  Connect(true, m_pFrame, SIGNAL(ChangeTrailsTime(int)), this,
          SLOT(Frame_ChangeTrailsTime(int)));
  Connect(true, m_pFrame, SIGNAL(AcquireTarget(double, double)), this,
          SIGNAL(AcquireTarget(double, double)));
}

//-----------------------------------------------------------------------------
tTabPPI::~tTabPPI() {
  m_Timer.stop();

  delete m_pController;
  delete m_pFrame;
  delete m_pImage;
}

//-----------------------------------------------------------------------------
void tTabPPI::OnConnect() {
  ui.tabPPI->setEnabled(true);
  m_Timer.setInterval(50);
  m_Timer.start();
}

//-----------------------------------------------------------------------------
void tTabPPI::OnDisconnect() {
  m_Timer.stop();
  ui.tabPPI->setEnabled(false);
}

//-----------------------------------------------------------------------------
void tTabPPI::Frame_ChangeTrailsTime(int secs) {
  m_pController->SetTrailsTime(secs);
}

//-----------------------------------------------------------------------------
void tTabPPI::Timer_timeout() {
  if (m_pFrame) m_pFrame->update();
}

//-----------------------------------------------------------------------------
void tTabPPI::OnUpdateSpoke(
    const Navico::Protocol::NRP::Spoke::t9174Spoke* pSpoke) {
  QMutexLocker locker(tQCustomFrame::getImageMutex());

  if (m_pImage == nullptr) {
    m_pImage = new QImage(2 * pSpoke->header.nOfSamples,
                          2 * pSpoke->header.nOfSamples, QImage::Format_RGB32);
    m_pImage->fill(0);
    m_pFrame->setImage(m_pImage);

    m_pController->SetFrameBuffer(
        (intptr_t)m_pImage->scanLine(0), m_pImage->size().width(),
        m_pImage->size().height(), &gNavicoLUT, m_pImage->size().width() / 2,
        m_pImage->size().height() / 2);
    m_pController->SetTrailsTime(-1);
  }

  uint32_t _fullrange_m =
      Navico::Protocol::NRP::Spoke::GetSampleRange_mm(pSpoke->header) *
      pSpoke->header.nOfSamples * 2 / 1000;
  float _Bearing_deg = (pSpoke->header.compassInvalid == 0)
                           ? CompassToDegrees(pSpoke->header.spokeCompass)
                           : 0.0f;
  m_pFrame->SetFullRange_m(_fullrange_m);
  m_pFrame->SetBearing_deg(_Bearing_deg);
  m_pController->Process(pSpoke);
}

//-----------------------------------------------------------------------------
//  tQBScanFrame Implementation
//-----------------------------------------------------------------------------
void tQBScanFrame::convertXYtoSD(int x, int y, double& s, double& d) {
  s = (getImage()->height() * y) / height();
  d = (360.0 * x) / width();
}

//-----------------------------------------------------------------------------
void tQBScanFrame::convertSDtoXY(double s, double d, int& x, int& y) {
  y = static_cast<int>(((s * height()) / getImage()->height()) + 0.5);
  x = static_cast<int>((d * width() / 360.0) + 0.5);
}

//-----------------------------------------------------------------------------
void tQBScanFrame::DrawOverlay(QPainter& painter, const tOverlay* pOverlay) {
  int width = this->width();
  int height = this->height();

  double x1 = (static_cast<double>(pOverlay->startBearing_deg) / 360.0) - 0.5;
  double x2 = (static_cast<double>(pOverlay->endBearing_deg) / 360.0) - 0.5;
  double y1 =
      (static_cast<double>(pOverlay->startRange_m) / m_FullRange_m) - 0.5;
  double y2 = (static_cast<double>(pOverlay->endRange_m) / m_FullRange_m) - 0.5;

  painter.setPen(pOverlay->lineColour);
  painter.setBrush(QBrush(pOverlay->fillColour));

  double leftBound = -0.5;
  double rightBound = 0.5;

  if (x2 >= x1) {
    painter.drawRect(static_cast<int>(x1 * width),
                     static_cast<int>(y1 * height),
                     static_cast<int>((x2 - x1) * width),
                     static_cast<int>((y2 - y1) * height));
  } else {
    painter.drawRect(static_cast<int>(leftBound * width),
                     static_cast<int>(y1 * height),
                     static_cast<int>((x2 - leftBound) * width),
                     static_cast<int>((y2 - y1) * height));
    painter.drawRect(static_cast<int>(x1 * width),
                     static_cast<int>(y1 * height),
                     static_cast<int>((rightBound - x1) * width),
                     static_cast<int>((y2 - y1) * height));
  }
}

//-----------------------------------------------------------------------------
//  tTabBScan Implementation
//-----------------------------------------------------------------------------
tTabBScan::tTabBScan(Ui::GUIDemoClass& myUI, tTargetLocation* pTargets,
                     unsigned maxTargets, QObject* pParent,
                     tOverlayManager& overlayManager)
    : QObject(pParent),
      m_pFrame(nullptr),
      m_pImage(nullptr),
      m_NumSamples(0),
      ui(myUI) {
  m_pFrame = new tQBScanFrame(pTargets, maxTargets, ui.tabBScan, nullptr,
                              overlayManager);
  ui.verticalLayout_tabBscan->addWidget(m_pFrame);
  Connect(true, &m_Timer, SIGNAL(timeout()), this, SLOT(Timer_timeout()));
  Connect(true, m_pFrame, SIGNAL(AcquireTarget(double, double)), this,
          SIGNAL(AcquireTarget(double, double)));
}

//-----------------------------------------------------------------------------
tTabBScan::~tTabBScan() {
  m_Timer.stop();

  delete m_pFrame;
  delete m_pImage;
}

//-----------------------------------------------------------------------------
void tTabBScan::OnConnect() {
  ui.tabBScan->setEnabled(true);
  m_Timer.setInterval(50);
  m_Timer.start();
}

//-----------------------------------------------------------------------------
void tTabBScan::OnDisconnect() {
  m_Timer.stop();
  ui.tabBScan->setEnabled(false);
}

//-----------------------------------------------------------------------------
void tTabBScan::Timer_timeout() {
  if (m_pFrame) m_pFrame->update();
}

//-----------------------------------------------------------------------------
void tTabBScan::OnUpdateSpoke(
    const Navico::Protocol::NRP::Spoke::t9174Spoke* pSpoke) {
  QMutexLocker locker(tQCustomFrame::getImageMutex());
  const uint32_t cSpokesPerRevolution = 2048;
  if (m_pImage == nullptr) {
    m_NumSamples = pSpoke->header.nOfSamples;

    // create the correct size of the frame buffer
    m_pImage = new QImage(cSpokesPerRevolution, static_cast<int>(m_NumSamples),
                          QImage::Format_RGB32);
    m_pImage->fill(0);
    m_pFrame->setImage(m_pImage);

    m_NumSamples = m_NumSamples / 2;
  }

  assert(pSpoke->header.bitsPerSample ==
         4);  // only 4 bits sample are supported

  uint32_t* pRawImage = (uint32_t*)m_pImage->scanLine(0);
  uint32_t azimuth = (pSpoke->header.spokeAzimuth >> 1) &
                     0x7ff;  // to be sure to have a value between 0-2047

  for (unsigned r = 0; r < m_NumSamples; ++r) {
    *(pRawImage + azimuth + ((r << 1)) * cSpokesPerRevolution) =
        gNavicoLUT.GetColour(((pSpoke->data[r]) & 0xf));
    *(pRawImage + azimuth + ((r << 1) + 1) * cSpokesPerRevolution) =
        gNavicoLUT.GetColour(((pSpoke->data[r] >> 4) & 0xf));
  }

  m_pFrame->SetFullRange_m(
      Navico::Protocol::NRP::Spoke::GetSampleRange_mm(pSpoke->header) *
      pSpoke->header.nOfSamples / 1000);
}
