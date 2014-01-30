/***************************************************************************
 *   Copyright (c) 2013 Andrew Robinson <andrewjrobinson@gmail.com>        *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef CAMMANAGER_H_
#define CAMMANAGER_H_

#include <boost/thread.hpp>

#include <queue>
#include <QObject>
#include <App/Document.h>
#include <App/DocumentObjectGroup.h>

#include "TPG/TPG.h"

namespace Cam {

/**
 * The CamManager is the master of all knowledge in the Cam Workbench.
 *
 * It provides a single point of communication with all aspects of the
 * workbench.  It provides signals and slots so that the UI (though the
 * UIManager) can interact with the data.
 * TODO: remove QObject super-class (i.e. use boost signals instead of QT ones)
 */
class CamExport CamManagerInst : public QObject {

	Q_OBJECT

protected:
	static CamManagerInst* _pcSingleton;

	// TPG Running Thread //
	/// Storage class for TPG's to run
	class TPGRunnerItem {
	public:
		TPGRunnerItem(TPG* tpg, Settings::TPGSettings* settings, TPGFeature *tpgFeature) {this->tpg = tpg; this->settings = settings; this->tpgFeature = tpgFeature;}
		~TPGRunnerItem() { if (this->settings) this->settings->release(); }
		TPG* tpg;
		Settings::TPGSettings* settings;
		TPGFeature* tpgFeature;
	};
	/// the thread that processes tpg run requests
	boost::thread* tpgRunnerThread;
	/// queue to store TPG's before they are run
	std::queue<TPGRunnerItem*> tpgRunnerQueue;
	/// mutex to protect access to tpgRunnerQueue
	boost::mutex tpgRunnerQueueMutex;
	/// flag to tell thread to stay alive
	bool tpgRunnerKeepAlive;
	/// method to add a TPG to the TPG Runner Queue
	bool queueTPGRun(TPG* tpg, Settings::TPGSettings* settings, TPGFeature *tpgFeature);
	/// the main function used for the tpgRunnerThread
	void tpgRunnerThreadMain();

public:

	CamManagerInst();
	virtual ~CamManagerInst();

	// singleton manipators
	static CamManagerInst& instance(void);
	static void destruct (void);

	// External API (for GUI)
	//TODO: make these functions available to Python
	/**
	 * Adds a TPG to the processing queue given its name.  If document is NULL
	 * then the active document will be used.
	 */
	bool runTPGByName(const char *FeatName, App::Document* document = NULL);

	bool runPostProcessByName(const char *FeatName, App::Document *document = NULL);

	// CLI Internal API (i.e. Non-GUI)
	/**
	 * Updates the progress of TPG processing
	 */
	void updateProgress(QString tpgid, TPG::State state, int progress);
	/**
	 * Adds a toolpath to the document under the given tpg.
	 */
	void addToolPath(TPGFeature* tpgFeature, ToolPath *toolPath);

	/**
	 * Create a new ToolFeature in the active document under the group (if provided
	 * otherwise root).
	 */
	bool ToolFeature(App::DocumentObjectGroup *docObjGroup = NULL);

    /**
     * Create a new MachineFeature in the active document under the group (if provided
     * otherwise root).
     */
    bool MachineFeature(App::DocumentObjectGroup *docObjGroup = NULL);

public Q_SLOTS:
//  void addTPG(Cam::TPGDescriptor *tpg);
//  void reloadTPGs();

Q_SIGNALS:
	/**
	 * Signalled when a TPG state changes (or progess changes).
	 *
	 * @param tpgid, QString, the id of the TPG that has changed
	 * @param state, TPG::State (enum), the new status
	 * @param progress, int, the total percentage progressed (0 <= progress <= 100)
	 */
	void updatedTPGState(QString tpgid, Cam::TPG::State state, int progress);
};

/// Get the global instance
inline CamManagerInst& CamManager(void)
{
    return CamManagerInst::instance();
}

} /* namespace Cam */
#endif /* CAMMANAGER_H_ */
