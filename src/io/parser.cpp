/*
** parser.cpp
**
** Copyright © Beyond Skyrim Development Team, 2017.
** This file is part of OPENCK (https://github.com/Beyond-Skyrim/openck)
**
** OpenCK is free software; this file may be used under the terms of the GNU
** General Public License version 3.0 or later as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** OpenCK is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** Please review the following information to ensure the GNU General Public
** License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** You should have received a copy of the GNU General Public License version
** 3.0 along with OpenCK; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** Created Date: 08-Jul-2017
*/

#include "parser.h"
#include "formfactory.h"

namespace io
{
    /**
     * Construct the parser and ensure we can't accidently delete stuff via pointers.
     * This exists solely to create an instance for getParser.
     * @brief Construct the parser.
     * @see Parser::getParser()
     */
    Parser::Parser()
    {
        factory = new FormFactory();
    }

    /**
     * Parses a list of .esm and .esp files (based on file path).
     * Defaults activePath to the first element of the list.
     * @brief Parses a list of .esm and .esp files.
     * @param list The list of files to be parsed.
     * @see Parser::parse(QStringList,QString)
     */
    void Parser::parse(QStringList list)
    {
        qDebug() << "No active file, defaulting.";
        warn("There is no active file. Defaulting Active File to first selected element.");
        parse(list,list.at(0));
    }

    /**
     * Parses a list of .esm and .esp files (based on file path) with a specified active file.
     * @brief Parses a list of .esm and .esp files with an active file.
     * @param list The list of file paths to be parsed.
     * @param activePath the Active File (the file on which changes are applied to).
     */
    void Parser::parse(QStringList list, QString activePath)
    {   
        for(int i = 0; i < list.size(); i++) {
            QFile file(list.at(i));
            QFileInfo info(file.fileName());
            QString name(info.fileName());
            emit addFile(name);

            if(!file.open(QIODevice::ReadOnly)) {
                warn(name.append(" could not be opened."));
                continue;
            }

            in.setDevice(&file);
            io::Reader& r = Reader(&in);
            int j = 0;

            while (true) { //Loop condition temporary
                quint32 type = r.readType();

                    if (type == 'GRUP') {
                        readGroupHeader(r);
                    }
                    else if (type == 'FACT') {
                        break;
                    }
                    else {
                        esx::Form* formHeader = readRecordHeader(r, type);
                        esx::Form* newForm = factory->createForm(*formHeader, r);
                        newForm->addForm();
                        delete formHeader;
                    }

                j++;
            }
        }

        emit updateFileModel();
    }

    /**
     * Construct a form by reading header values. This can be copied to a new form.
     * @brief Make a new form and read header.
     * @param r Provides raw data read functionality.
     * @param type Type code of form.
     * @return
     */
    esx::Form *Parser::readRecordHeader(io::Reader& r, quint32 type)
    {
        esx::Form *form = new esx::Form();
        form->readHeader(r, type);

        return form;
    }

    /**
     * Read a group header. This is currently not needed by the parser.
     * @brief Read group header values.
     * @param r Provides raw data read functionality.
     */
    void Parser::readGroupHeader(io::Reader& r)
    {
        //Temporary -- skip groups
        for (int i = 0; i < 20; ++i) {
            r.read<quint8>();
        }
    }

    /**
     * Gives a warning to the user with a given string.
     * @brief Gives a warning to the user with a given string.
     * @param message Message to be displayed in the warning box.
     */
    void Parser::warn(QString message)
    {
        QMessageBox* msgBox = new QMessageBox;
        msgBox->setSizeIncrement(600, 400);
        msgBox->setText(message);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setWindowIcon(QIcon(":/openck32x32.png"));
        msgBox->exec();
    }

    /**
     * Get the instance of the parser.
     * @brief Get the instance of the parser.
     * @return The instance of the parser.
     */
    Parser& Parser::getParser()
    {
        static Parser parser;
        return parser;
    }

    /**
     * Initialise the parser with a file and form model to use in signal/slot connections.
     * @brief Initialise the parser.
     * @param fileModel Pointer to file model instance.
     * @param formModel Pointer to form model instance.
     */
    void Parser::init(models::FileModel* fileModel, models::FormModel* formModel)
    {
        this->fileModel = fileModel;
        this->formModel = formModel;

        connect(this, &Parser::addFile,
                this->fileModel, &models::FileModel::insertFile);
    }

    /**
     * Return instance of file model.
     * @brief Return instance of file model.
     * @return Instance of file model.
     */
    models::FileModel& Parser::getFileModel()
    {
        return *fileModel;
    }

    /**
     * Return instance of fomr model.
     * @brief Return instance of form model.
     * @return Instance of form model.
     */
    models::FormModel& Parser::getFormModel()
    {
        return *formModel;
    }
}
