#if !defined __MODELMANAGER_H__
#define __MODELMANAGER_H__

class Animation;
class Model;

class ModelManager
{
private:
    typedef boost::unordered_map<std::string, boost::shared_ptr<Animation> > AnimationMap;
    typedef boost::unordered_map<std::string, boost::shared_ptr<Model> > ModelMap;

    static Logger& logger;

public:
    static ModelManager& instance();

public:
    virtual ~ModelManager() throw();

public:
    bool load_animation(const boost::filesystem::path& path, const std::string& model, const std::string& name);
    boost::shared_ptr<Animation> animation(const std::string& model, const std::string& name) const;

    bool load_model(const boost::filesystem::path& path, const std::string& name);
    boost::shared_ptr<Model> model(const std::string& name) const;

private:
    AnimationMap _animations;
    ModelMap _models;

private:
    ModelManager();
    DISALLOW_COPY_AND_ASSIGN(ModelManager);
};

#endif
