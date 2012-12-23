#include "pch.h"
#include <iostream>
#include "common.h"
#include "Animation.h"
#include "MD5Animation.h"
#include "MD5Model.h"
#include "Model.h"
#include "ModelManager.h"

Logger& ModelManager::logger(Logger::instance("md5mv.ModelManager"));

ModelManager& ModelManager::instance()
{
    static boost::shared_ptr<ModelManager> model_manager;
    if(!model_manager) {
        model_manager.reset(new ModelManager());
    }
    return *model_manager;
}

ModelManager::ModelManager()
{
}


ModelManager::~ModelManager() throw()
{
}

bool ModelManager::load_animation(const boost::filesystem::path& path, const std::string& model, const std::string& name)
{
    boost::shared_ptr<Animation> animation;
    if(boost::filesystem::exists(model_dir() / path / (name + MD5Animation::extension()))) {
        animation.reset(new MD5Animation(name));
    } else if(boost::filesystem::exists(model_dir() / path / (name + Animation::extension()))) {
        animation.reset(new Animation(name));
    }

    if(!animation) {
        LOG_ERROR("Could not find animation '" << name << "'!" << std::endl);
        return false;
    }

    if(!animation->load(path)) {
        LOG_ERROR("Error loading animation '" << name << "'!" << std::endl);
        return false;
    }
    animation->build_skeletons();

    _animations[model + "_" + name] = animation;
    return true;
}

boost::shared_ptr<Animation> ModelManager::animation(const std::string& model, const std::string& name) const
{
    try {
        return _animations.at(model + "_" + name);
    } catch(const std::out_of_range&) {
        LOG_WARNING("No such animation '" << name << "' for model '" << model << "'!" << std::endl);
        return boost::shared_ptr<Animation>();
    }
}

bool ModelManager::load_model(const boost::filesystem::path& path, const std::string& name)
{
    boost::shared_ptr<Model> model;
    if(boost::filesystem::exists(model_dir() / path / (name + MD5Model::extension()))) {
        model.reset(new MD5Model(name));
    } else if(boost::filesystem::exists(model_dir() / path / (name + Model::extension()))) {
        model.reset(new Model(name));
    }

    if(!model) {
        LOG_ERROR("Could not find model '" << name << "'!" << std::endl);
        return false;
    }

    if(!model->load(path)) {
        LOG_ERROR("Error loading model '" << name << "'!" << std::endl);
        return false;
    }

    if(!model->load_textures(path)) {
        LOG_ERROR("Error loading model textures!" << std::endl);
        return false;
    }

    _models[name] = model;
    return true;
}

boost::shared_ptr<Model> ModelManager::model(const std::string& name) const
{
    try {
        return _models.at(name);
    } catch(const std::out_of_range&) {
        return boost::shared_ptr<Model>();
    }
}
