#include "EncryptionAuthentication.h"

#include <Windows.h>
#include <Vikey.h>
#include <qdebug.h>
#include <qregularexpression.h>

void EncryptionAuthentication::setErrorMessage(int errorCode, QString message)
{
	m_messages[errorCode] = message;
}

EncryptionAuthentication::EncryptionAuthentication(
	unsigned long hardID,
	QString softID,
	QString productName,
	QDateTime expiredDateTime,
	QWidget * parent)
	:
	m_hardID(hardID),
	m_softID(softID),
	m_productName(productName),
	m_expiredDateTime(expiredDateTime),
	m_messageBox(QMessageBox::Critical, "Please check your USB key.", "", 
		QMessageBox::Cancel|QMessageBox::Retry, this),
	QWidget(parent)
{
	m_messages[NORMAL] = "This product will be expired at " +
		m_expiredDateTime.toString("hh:mm:ss dd/MMM/yyyy") + ".";
	m_messages[HAVING_KEY] = "Please insert your USB key, and make sure only one key inserted.";
	m_messages[SOFT_ID] = "Please insert USB key with right Soft ID " + m_softID;
	m_messages[HARD_ID] = "Please insert USB key with right Hard ID(Unique ID).";
	m_messages[PRODUCT_NAME] = "Please insert USB key for product's name is " +
		m_productName;
	m_messages[EXPIRED_DATE_TIME] = "This product has already been expired at " + 
		m_expiredDateTime.toString("hh:mm:ss dd/MMM/yyyy") +
		"." + "Please ask the manufacture for a renewed version. ";

	reloadViKey();
	
}

EncryptionAuthentication::~EncryptionAuthentication()
{
}

void EncryptionAuthentication::reloadViKey()
{
	DWORD dwCount;
	m_errorType = VikeyFind(&dwCount);
}

int EncryptionAuthentication::authenticationExec(int errorCode)
{

	bool loopFlag = true;
	int authenticationPass = SUCCESSFUL;
	int status;
	if (errorCode == NORMAL) {
		return SUCCESSFUL;
	}
	while (loopFlag) {
		QString message;
		authenticationPass = SUCCESSFUL;
		reloadViKey();

		if (errorCode & HAVING_KEY) {
			if (!authenticationByHavingKey()) {
				message += '\n';
				message += m_messages[HAVING_KEY];
				authenticationPass = FAIL;
			}
		}
		if (errorCode & SOFT_ID) {
			if (!authenticationBySoftID()) {
				message += '\n';
				message += m_messages[SOFT_ID];
				authenticationPass = FAIL;
			}
		}
		if (errorCode & HARD_ID) {
			if (!authenticationByHardID()) {
				message += '\n';
				message += m_messages[HARD_ID];
				authenticationPass = FAIL;
			}
		}
		if (errorCode & PRODUCT_NAME) {
			if (!authenticationByProductName()) {
				message += '\n';
				message += m_messages[PRODUCT_NAME];
				authenticationPass = FAIL;
			}
		}
		if (errorCode & EXPIRED_DATE_TIME) {
			if (!authenticationByExpiredDateTime()) {
				message += '\n';
				message += m_messages[EXPIRED_DATE_TIME];
				authenticationPass = FAIL;
			}
		}
		if (authenticationPass == SUCCESSFUL) {
			loopFlag = false;
			if (m_expiredDateTime != QDateTime()) {
				m_messageBox.setIcon(QMessageBox::Information);
				m_messageBox.setStandardButtons(QMessageBox::Ok);
				m_messageBox.setText(m_messages[NORMAL]);
				m_messageBox.exec();
			}

		}
		else {
			m_messageBox.setText(message);
			status = m_messageBox.exec();
			if (status == QMessageBox::Cancel) {
				loopFlag = false;
			}
		}
		
	}
	return authenticationPass;

}

void EncryptionAuthentication::setUserPassword(QString userPassword)
{
	m_userPassWord = userPassword;
}

void EncryptionAuthentication::setAdminPassword(QString adminPassword)
{
	m_adminPassWord = adminPassword;
}

int EncryptionAuthentication::authenticationBySoftID()
{
	if (m_softID.isEmpty()) {
		return SUCCESSFUL;
	}
	WORD index = 0;
	char szSoftID[8];

	if (!VikeyGetSoftIDString(index, szSoftID)) {
		QString _softId = QString(szSoftID);
		_softId.truncate(m_softID.size());
		if (_softId == m_softID) {
			return SUCCESSFUL;
		}
		else {
			return FAIL;
		}
	}
	else {
		return FAIL;
	}
}

int EncryptionAuthentication::authenticationByHardID()
{
	WORD index = 0;
	DWORD pdwHID;
	if (m_hardID == 0) {
		return SUCCESSFUL;
	}
	if (!VikeyGetHID(index, &pdwHID)) {
		if (pdwHID == m_hardID) {
			return SUCCESSFUL;
		}
		else {
			return FAIL;
		}
	}
	else {
		return FAIL;
	}

}
int EncryptionAuthentication::authenticationByProductName()
{
	if (m_productName.isEmpty()) {
		return SUCCESSFUL;
	}
	WORD index = 0;
	WCHAR szName[16];
	if (VikeyUserLogin(index, const_cast<char*>(m_userPassWord.toStdString().c_str())))
	{
		return FAIL;
	}

	if (!VikeyGetPtroductName(index, szName)) {
		VikeyLogoff(index);
		QString _productName = QString::fromWCharArray(szName);
		_productName.truncate(m_productName.size());
		if (_productName == m_productName ) {
			return SUCCESSFUL;
		}
		else {
			return FAIL;
		}
	}
	else {
		VikeyLogoff(index);
		return FAIL;
	}
}

int EncryptionAuthentication::authenticationByHavingKey()
{
	DWORD dwCount;
	if (!VikeyFind(&dwCount))
	{
		return SUCCESSFUL;
	}
	else if (dwCount > 1)
	{
		return FAIL;
	}
	else {
		return FAIL;
	}
}

int EncryptionAuthentication::authenticationByExpiredDateTime()
{
	// if m_expiredDateTime is not specified, it is the OS's dateTime
	// it will return successfule directly
	if (m_expiredDateTime == QDateTime()) {
		return SUCCESSFUL;
	}
	WORD Index = 0;
	//获取时钟型加密狗中的内部时间
	SVikeyTime pTime;
	if (!VikeyGetTime(Index, &pTime)) {
		char* time = reinterpret_cast<char*>(&pTime);
		QString year = QString::number(time[0]);
		QString month = QString::number(time[1]);
		QString day = QString::number(time[2]);
		QString hour = QString::number(time[3]);
		QString minute = QString::number(time[4]);
		QString second = QString::number(time[5]);

		QDateTime dateTimeOfKey(QDate(time[0] + 2000, time[1], time[2]),
			QTime(time[3], time[4], time[5]));

		if (dateTimeOfKey > m_expiredDateTime) {
			return FAIL;
		}
		else {
			return SUCCESSFUL;
		}
	}
	else {
		return FAIL;
	}



}
