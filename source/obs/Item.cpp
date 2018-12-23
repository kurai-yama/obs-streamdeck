/*
 * Plugin Includes
 */
#include "include/obs/Scene.hpp"
#include "include/obs/Item.hpp"

 /*
  * Qt Includes
  */
#include <QString>

/*
========================================================================================================
	Constructors / Destructor
========================================================================================================
*/

Item::Item(Scene* scene, uint16_t id, obs_sceneitem_t* item) :
	OBSStorable(id, obs_source_get_name(obs_sceneitem_get_source(item))),
	m_parentScene(scene),
	m_item(item) {
	m_source = obs_sceneitem_get_source(m_item);
}

Item::Item(Scene* scene, uint16_t id, std::string name) :
	OBSStorable(id, name),
	m_parentScene(scene) {
	m_source = obs_sceneitem_get_source(m_item);
}

Item::~Item() {
}

/*
========================================================================================================
	Accessors
========================================================================================================
*/

obs_sceneitem_t*
Item::item() const {
	return m_item;
}

void
Item::item(obs_sceneitem_t* item) {
	m_item = item;
	m_source = obs_sceneitem_get_source(item);
	m_name = obs_source_get_name(m_source);
}

const char*
Item::type() const {
	return "item";
}

bool
Item::visible() const {
	return obs_sceneitem_visible(m_item);
}