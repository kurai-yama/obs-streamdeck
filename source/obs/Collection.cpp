/*
 * Plugin Includes
 */
#include "include/obs/Collection.hpp"

/*
========================================================================================================
	Constructors / Destructor
========================================================================================================
*/

Collection::Collection(unsigned long long id, std::string name) :
	m_identifier(id),
	m_name(name),
	m_activeScene(nullptr) {
}

Collection::~Collection() {
}

/*
========================================================================================================
	Builders
========================================================================================================
*/

Collection*
Collection::buildFromMemory(Memory& memory) {
	unsigned long long id = 0;
	unsigned int namelen = 0;
	char collection_name[MAX_NAME_LENGTH];
	short nb_scenes = 0;

	if(memory == nullptr)
		return nullptr;

	memory.read((byte*)&id, sizeof(unsigned long long));
	memory.read((byte*)&namelen, sizeof(unsigned int));
	collection_name[namelen] = 0;
	memory.read(collection_name, namelen);
	memory.read((byte*)&nb_scenes, sizeof(short));

	Collection* collection = new Collection(id, collection_name);

	while(nb_scenes > 0) {
		size_t block_size = 0;
		memory.read((byte*)&block_size, sizeof(size_t));
		void* end_of_block = memory.tell() + block_size;
		Scene* scene = Scene::buildFromMemory(collection, memory);
		if(scene != nullptr) {
			collection->m_scenes.insert(
				std::map<unsigned long long, std::shared_ptr<Scene>>::value_type(
					scene->id(),
					scene
				)
			);
		}
		if(memory.tell() != end_of_block) {
			delete collection;
			collection = nullptr;
			nb_scenes = 0;
		}
		nb_scenes--;
	}

	return collection;
}

/*
========================================================================================================
	Serialization
========================================================================================================
*/

Memory
Collection::toMemory(size_t& size) const {

	std::vector<Memory> scene_blocks;

	/*BLOCK
		id (unsigned long long)
		namelen (unsigned int)
		name (namelen)
		nbscenes (short)
		foreach(scene)
			block_size (size_t)
			block_scene (block_size)
	*/
	unsigned int namelen = strlen(m_name.c_str());
	size_t total_size = sizeof(unsigned long long) + sizeof(unsigned int) + namelen + sizeof(short);
	short nb_scenes = m_scenes.size();
	for(auto iter = m_scenes.begin(); iter != m_scenes.end(); iter++) {
		total_size += sizeof(size_t);
		scene_blocks.push_back(iter->second->toMemory(total_size));
	}

	Memory block(total_size);
	block.write((byte*)&m_identifier, sizeof(unsigned long long));
	block.write((byte*)&namelen, sizeof(unsigned int));
	block.write((byte*)m_name.c_str(), namelen);
	block.write((byte*)&nb_scenes, sizeof(short));

	for(auto iter = scene_blocks.begin(); iter < scene_blocks.end(); iter++) {
		size_t sc_size = iter->size();
		block.write((byte*)&sc_size, sizeof(size_t));
		block.write(*iter, iter->size());
	}

	size += total_size;
	return block;
}

/*
========================================================================================================
	Scenes Helpers
========================================================================================================
*/

void
Collection::extractFromOBSScenes(unsigned long long& next_scene_identifier) {
	std::map<std::string, Scene*> scenes;
	for(auto scene_it = m_scenes.begin(); scene_it != m_scenes.end(); scene_it++) {
		scenes.insert(std::map<std::string, Scene*>::value_type(
			scene_it->second->name(),
			scene_it->second.get()
		));
	}

	obs_frontend_set_current_scene_collection(m_name.c_str());
	obs_frontend_source_list obs_scenes = {};
	obs_frontend_get_scenes(&obs_scenes);
	for(size_t i = 0; i < obs_scenes.sources.num; i++) {
		obs_source_t* obs_source = obs_scenes.sources.array[i];
		std::string obs_source_name = obs_source_get_name(obs_source);
		Scene* scene = nullptr;
		if(scenes.size() > 0) {
			auto scene_it = scenes.find(obs_source_name);
			if(scene_it != scenes.end()) {
				scene = scene_it->second;
				scene->source(obs_source);
				scenes.erase(scene_it);
				next_scene_identifier = std::max<size_t>(next_scene_identifier, scene->id());
			}
		}
		if(scene == nullptr) {
			++next_scene_identifier;
			m_scenes.insert(std::map<unsigned long long, std::shared_ptr<Scene>>::value_type(
				next_scene_identifier,
				new Scene(this, next_scene_identifier, obs_source)
			));
		}
	}
	obs_frontend_source_list_free(&obs_scenes);

	if(scenes.size() > 0) {
		for(auto scene_it = scenes.begin(); scene_it != scenes.end(); scene_it++) {
			m_scenes.erase(scene_it->second->id());
		}
	}
}

obs::scene_event
Collection::updateScenes(
	unsigned long long& next_scene_identifier,
	std::shared_ptr<Scene>& scene_updated
) {
	obs::scene_event event = obs::scene_event::SCENES_LIST_BUILD;

	std::map<std::string, std::shared_ptr<Scene>> scenes;
	for(auto iter = m_scenes.begin(); iter != m_scenes.end(); iter++) {
		scenes[iter->second->name()] = iter->second;
	}

	obs_frontend_source_list obs_scenes = {};
	obs_frontend_get_scenes(&obs_scenes);

	int j = -1;
	for(int i = 0; i < obs_scenes.sources.num; i++) {
		std::string scene_name(obs_source_get_name(obs_scenes.sources.array[i]));
		auto iter = scenes.find(scene_name);

		if(iter == scenes.end()) {
			j = i;
		}
		else {
			scenes.erase(iter);
		}
	}

	if(j != -1) {
		// New scenes
		if(scenes.size() == 0) {
			++next_scene_identifier;
			m_scenes.insert(
				std::map<unsigned long long, std::shared_ptr<Scene>>::value_type(
					next_scene_identifier,
					new Scene(this, next_scene_identifier, obs_scenes.sources.array[j])
				)
			);
			scene_updated = m_scenes.find(next_scene_identifier)->second;
			m_activeScene = nullptr;
			event = obs::scene_event::SCENE_ADDED;
		}
		// Update collection
		else if(scenes.size() == 1) {
			scene_updated = scenes.begin()->second;
			scene_updated->source(obs_scenes.sources.array[j]);
			event = obs::scene_event::SCENE_RENAMED;
		}
	}
	else {
		scene_updated = scenes.begin()->second;
		m_scenes.erase(scenes.begin()->second->id());
		m_activeScene = nullptr;
		event = obs::scene_event::SCENE_REMOVED;
	}

	obs_frontend_source_list_free(&obs_scenes);
	return event;
}

Scene*
Collection::activeScene() const {
	obs_source_t* current_scene_src = obs_frontend_get_current_scene();
	const char* current_scene = obs_source_get_name(current_scene_src);
	obs_source_release(current_scene_src);

	if(m_activeScene) {
		if(m_activeScene->name().compare(current_scene) == 0)
			return m_activeScene;
		m_activeScene = nullptr;
	}

	auto scene_it = m_scenes.begin();
	while(scene_it != m_scenes.end() && m_activeScene == nullptr) {
		if(scene_it->second->name().compare(current_scene) == 0) {
			m_activeScene = scene_it->second.get();
		}
		scene_it++;
	}

	return m_activeScene;
}

/*
========================================================================================================
	Accessors
========================================================================================================
*/

std::string
Collection::name() const {
	return m_name;
}

void
Collection::name(std::string new_name) {
	m_name = new_name;
}

unsigned long long
Collection::id() const {
	return m_identifier;
}

Scenes
Collection::scenes() const {
	Scenes scenes;
	for(auto iter = m_scenes.begin(); iter != m_scenes.end(); iter++)
		scenes.push_back(const_cast<Scene*>(iter->second.get()));
	return scenes;
}