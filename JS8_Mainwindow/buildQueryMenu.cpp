
/** \file
 * @brief member function of the UI_Constructor class
 *  builds the callsign query menu
 */

#include "JS8_UI/mainwindow.h"

void UI_Constructor::buildQueryMenu(QMenu *menu, QString call) {
    bool isAllCall = isAllCallIncluded(call);

    // for now, we're going to omit displaying the call...delete this if we want
    // the other functionality
    call = "";

    auto grid = m_config.my_grid();

    bool emptyInfo = m_config.my_info().isEmpty();
    bool emptyGrid = m_config.my_grid().isEmpty();

    auto callAction = menu->addAction(
        tr("Send a directed message to selected callsign"));
    connect(callAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 ").arg(selectedCall), true);
    });

    menu->addSeparator();

    auto sendReplyAction = menu->addAction(
        QString("%1 Reply - %2").arg(call).arg(tr("Send reply message to selected callsign")).trimmed());
    connect(sendReplyAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        auto message = m_config.reply_message();
        message = replaceMacros(message, buildMacroValues(), true);
        addMessageText(QString("%1 %2").arg(selectedCall).arg(message), true);
    });

    auto sendSNRAction = menu->addAction(
        QString("%1 SNR - %2").arg(call).arg(tr("Send a signal report to the selected callsign")).trimmed());
    sendSNRAction->setEnabled(m_callActivity.contains(callsignSelected()));
    connect(sendSNRAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        if (!m_callActivity.contains(selectedCall)) {
            return;
        }

        auto d = m_callActivity[selectedCall];
        addMessageText(QString("%1 SNR %2")
                           .arg(selectedCall)
                           .arg(Varicode::formatSNR(d.snr)),
                       true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto infoAction = menu->addAction(
        QString("%1 INFO - %2").arg(call).arg(tr("Send my station information")).trimmed());
    infoAction->setDisabled(emptyInfo);
    connect(infoAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(
            QString("%1 INFO %2").arg(selectedCall).arg(m_config.my_info()),
            true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto gridAction = menu->addAction(
        QString("%1 GRID %2 - %3").arg(call).arg(grid).arg(tr("Send my current station Maidenhead grid locator")).trimmed());
    gridAction->setDisabled(emptyGrid);
    connect(gridAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(
            QString("%1 GRID %2").arg(selectedCall).arg(m_config.my_grid()),
            true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    menu->addSeparator();

    auto snrQueryAction = menu->addAction(
        QString("%1 SNR? - %2").arg(call).arg(tr("What is my signal report?")).trimmed());
    snrQueryAction->setDisabled(isAllCall);
    connect(snrQueryAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 SNR?").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto infoQueryAction =
        menu->addAction(QString("%1 INFO? - %2").arg(call).arg(tr("What is your station information?")).trimmed());
    infoQueryAction->setDisabled(isAllCall);
    connect(infoQueryAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 INFO?").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto gridQueryAction =
        menu->addAction(QString("%1 GRID? - %2").arg(call).arg(tr("What is your current grid locator?")).trimmed());
    gridQueryAction->setDisabled(isAllCall);
    connect(gridQueryAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 GRID?").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto stationIdleQueryAction = menu->addAction(
        QString("%1 STATUS? - %2").arg(call).arg(tr("What is your station status message?")).trimmed());
    stationIdleQueryAction->setDisabled(isAllCall);
    connect(stationIdleQueryAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 STATUS?").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto heardQueryAction = menu->addAction(
        QString("%1 HEARING? - %2").arg(call).arg(tr("What are the stations are you hearing? (Top 4 ranked by most recently heard)")).trimmed());
    heardQueryAction->setDisabled(isAllCall);
    connect(heardQueryAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 HEARING?").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

#if 0
    auto retransmitAction = menu->addAction(QString("%1|[MESSAGE] - Please ACK and retransmit the following message").arg(call).trimmed());
    retransmitAction->setDisabled(isAllCall);
    connect(retransmitAction, &QAction::triggered, this, [this](){

        QString selectedCall = callsignSelected();
        if(selectedCall.isEmpty()){
            return;
        }

        addMessageText(QString("%1|[MESSAGE]").arg(selectedCall), true, true);
    });
#endif

    auto alertAction = menu->addAction(
        QString("%1>[MESSAGE] - %2").arg(call).arg(tr("Please relay this message to its destination")).trimmed());
    alertAction->setDisabled(isAllCall);
    connect(alertAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1>[MESSAGE]").arg(selectedCall), true, true);
    });

    auto msgAction = menu->addAction(
        QString("%1 MSG [MESSAGE] - %2").arg(call).arg(tr("Please store this message in your inbox")).trimmed());
    msgAction->setDisabled(isAllCall);
    connect(msgAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 MSG [MESSAGE]").arg(selectedCall), true,
                       true);
    });

    auto msgToAction = menu->addAction(
        QString("%1 MSG TO:[CALLSIGN] [MESSAGE] - %2").arg(call).arg(tr("Please store this message at your station for later retreival by [CALLSIGN]")).trimmed());
    msgToAction->setDisabled(isAllCall);
    connect(msgToAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(
            QString("%1 MSG TO:[CALLSIGN] [MESSAGE]").arg(selectedCall), true,
            true);
    });

    auto qsoQueryAction = menu->addAction(
        QString("%1 QUERY CALL [CALLSIGN]? - %2").arg(call).arg(tr("Please acknowledge you can communicate directly with [CALLSIGN]")).trimmed());
    connect(qsoQueryAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 QUERY CALL [CALLSIGN]?").arg(selectedCall),
                       true, true);
    });

    auto qsoQueryMsgsAction = menu->addAction(
        QString("%1 QUERY MSGS - %2").arg(call).arg(tr("Do you have any messages for me?")).trimmed());
    connect(qsoQueryMsgsAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 QUERY MSGS").arg(selectedCall), true, true);
    });

    auto qsoQueryMsgAction =
        menu->addAction(QString("%1 QUERY MSG [ID] - %2").arg(call).arg(tr("Please deliver the complete message identified by ID")).trimmed());
    connect(qsoQueryMsgAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 QUERY MSG [ID]").arg(selectedCall), true,
                       true);
    });

    menu->addSeparator();

    auto agnAction = menu->addAction(
        QString("%1 AGN? - %2").arg(call).arg(tr("Please repeat your last transmission")).trimmed());
    connect(agnAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 AGN?").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto qslQueryAction = menu->addAction(
        QString("%1 QSL? - %2").arg(call).arg(tr("Did you receive my last transmission?")).trimmed());
    connect(qslQueryAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 QSL?").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto qslAction = menu->addAction(
        QString("%1 QSL - %2").arg(call).arg(tr("I confirm I received your last transmission")).trimmed());
    connect(qslAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 QSL").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto yesAction = menu->addAction(
        QString("%1 YES - %2").arg(call).arg(tr("I confirm your last inquiry")).trimmed());
    connect(yesAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 YES").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto noAction =
        menu->addAction(QString("%1 NO - %2").arg(call).arg(tr("I do not confirm your last inquiry")).trimmed());
    connect(noAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 NO").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto hwAction = menu->addAction(
        QString("%1 HW CPY? - %2").arg(call).arg(tr("How do you copy?")).trimmed());
    connect(hwAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 HW CPY?").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto rrAction = menu->addAction(
        QString("%1 RR - %2").arg(call).arg(tr("Roger. Received. I copy.")).trimmed());
    connect(rrAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 RR").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto fbAction =
        menu->addAction(QString("%1 FB - %2").arg(call).arg(tr("Fine Business")).trimmed());
    connect(fbAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 FB").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto sevenThreeAction = menu->addAction(
        QString("%1 73 - %2").arg(call).arg(tr("I send my best regards")).trimmed());
    connect(sevenThreeAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 73").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto skAction =
        menu->addAction(QString("%1 SK - %2").arg(call).arg(tr("End of contact")).trimmed());
    connect(skAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 SK").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });

    auto ditDitAction = menu->addAction(
        QString("%1 DIT DIT - %2").arg(call).arg(tr("End of contact / Two bits")).trimmed());
    connect(ditDitAction, &QAction::triggered, this, [this]() {
        QString selectedCall = callsignSelected();
        if (selectedCall.isEmpty()) {
            return;
        }

        addMessageText(QString("%1 DIT DIT").arg(selectedCall), true);

        if (m_config.transmit_directed())
            toggleTx(true);
    });
}
