#pragma once

/*
 * STL Includes
 */
#include <fstream>
#include <map>
#include <vector>
#include <memory>

/*
 * OBS Includes
 */
#include <obs.h>
#include <obs-frontend-api/obs-frontend-api.h>
#include <obs-module.h>

/*
 * Plugin Includes
 */
#include "include/events/EventTrigger.hpp"
#include "include/common/Memory.hpp"
#include "include/obs/OBSStorage.hpp"
#include "include/obs/OBSEvents.hpp"
#include "include/obs/Collection.hpp"

#include "include/triggers/FrontendEventTrigger.hpp"
#include "include/triggers/SaveEventTrigger.hpp"
#include "include/triggers/OutputEventTrigger.hpp"
#include "include/triggers/SceneItemEventTrigger.hpp"

/*
========================================================================================================
	Types Definitions
========================================================================================================
*/

class OBSManager {

	/*
	====================================================================================================
		Instance Data Members
	====================================================================================================
	*/
	private:

		OBSStorage<Collection> m_collections;

		FrontendEventTrigger m_frontendEvent;

		SaveEventTrigger m_saveEvent;

		OutputEventTrigger m_outputEvent;

		SceneItemEventTrigger m_sceneitemEvent;

		mutable Collection* m_activeCollection;

		uint16_t m_lastCollectionID;

		bool m_isLoadingCollection;

	/*
	====================================================================================================
		Constructors / Destructor
	====================================================================================================
	*/
	public:

		OBSManager();

		~OBSManager();

	/*
	====================================================================================================
		Instance Methods
	====================================================================================================
	*/
	public:

		void
		addEventHandler(const obs::frontend::event event, EventObserver<obs::frontend::event>* handler);

		void
		addEventHandler(const obs::save::event event, EventObserver<obs::save::event>* handler);

		void
		addEventHandler(const obs::output::event event, EventObserver<obs::output::event>* handler);

		void
		addEventHandler(const obs::item::event event, EventObserver<obs::item::event>* handler);

		void
		registerOutput(obs_output_t* output);

		void
		unregisterOutput(obs_output_t* output);

		void
		registerScene(const Scene* scene);

		void
		unregisterScene(const Scene* scene);

		void
		registerItem(const Item* item);

		void
		unregisterItem(const Item* item);

		void
		cleanRegisteredScenes();

		void
		loadCollections(OBSStorage<Collection>& collections, const uint16_t last_collection_id);

		bool
		isLoadingCollection() const;

		obs::collection::event
		updateCollections(std::shared_ptr<Collection>& collection_updated);

		void
		makeActive();

		bool
		switchCollection(Collection* collection);

		bool
		switchCollection(uint16_t id);

		bool
		switchCollection(const char* name);

		Collection*
		activeCollection() const;

		Collection*
		collection(uint16_t id) const;

		Collections
		collections() const;

};