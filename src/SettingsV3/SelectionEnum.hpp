// THIS IS UNUSED I DUNNO HOW TO MATJSON :Skull:
#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

struct MyComplexSettingValue {
    int m_currentMemeClick = 0;
    int m_currentClick = 0;
    int m_tab = 0;
    std::string CustomSoundPath;

    // Comparison operator
    bool operator==(MyComplexSettingValue const& other) const = default;

    // Implicit conversion to std::string
      operator std::string() const {
       return matjson::makeObject({
            {"Tab", m_tab},
            {"Current_Sound_Useful", m_currentClick},
            {"Current_Sound_Meme", m_currentMemeClick},
            {"Custom_Sound_Path", CustomSoundPath}
        }).dump(matjson::NO_INDENTATION);
    }

    // Constructors
    MyComplexSettingValue() = default;

    MyComplexSettingValue(int i1, int i2, int i3, std::string_view val)
        : m_tab(i1), m_currentClick(i2), m_currentMemeClick(i3), CustomSoundPath(val) {}

    MyComplexSettingValue(MyComplexSettingValue const&) = default;
};

template <>
struct matjson::Serialize<MyComplexSettingValue> {

    static Result<MyComplexSettingValue> fromJson(matjson::Value const& v) {
        GEODE_UNWRAP_INTO(std::string x, v.asString());
        if (x == "") {
           return Ok(MyComplexSettingValue(0, 0, 0, " ")); 
        }
       try {
        auto value = matjson::parse(x).unwrap();
        return Ok(MyComplexSettingValue(
            value["Tab"].asInt().unwrap(), 
            value["Current_Sound_Useful"].asInt().unwrap(), 
            value["Current_Sound_Meme"].asInt().unwrap(), 
            value["Custom_Sound_Path"].asString().unwrap()
        ));
        } catch (const std::exception& e) {
            return Ok(MyComplexSettingValue(0, 0, 0, " "));
        }
    }

    static bool is_json(matjson::Value const& json) {
        return json.isString();
    }
};




class MyCustomSettingV3 : public SettingBaseValueV3<MyComplexSettingValue> {
public:
    static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json) {
        auto res = std::make_shared<MyCustomSettingV3>();
        auto root = checkJson(json, "selectionclicks");
        res->parseBaseProperties(key, modID, root);
        root.checkUnknownKeys();
        return root.ok(std::static_pointer_cast<SettingV3>(res));
    }

    SettingNodeV3* createNode(float width) override;
};

template <>
struct geode::SettingTypeForValueType<MyComplexSettingValue> {
    using SettingType = MyCustomSettingV3;
};

class MyCustomSettingNodeV3 : public SettingValueNodeV3<MyCustomSettingV3> {
protected:
    std::vector<CCMenuItemToggler*> m_toggles;

    bool init(std::shared_ptr<MyCustomSettingV3> setting, float width) {
        if (!SettingValueNodeV3::init(setting, width))
            return false;
        
        int count = 0;
        for (auto value : {
            std::make_pair(0, "Meme"),
            std::make_pair(1, "Useful"),
            std::make_pair(2, "Custom")
        }) {
            count+=40;
            auto offSpr = ButtonSprite::create(value.second, 40.f, true, "bigFont.fnt", "GJ_button_04.png", 20.f, 1.0f);
            offSpr->setOpacity(90);
            auto onSpr = ButtonSprite::create(value.second, 40.f, true, "bigFont.fnt", "GJ_button_01.png", 20.f, 1.0f);
            auto toggle = CCMenuItemToggler::create(
                offSpr, onSpr, this, menu_selector(MyCustomSettingNodeV3::onToggle)
            );
            toggle->m_notClickable = true;
            toggle->setTag(static_cast<int>(value.first));
            m_toggles.push_back(toggle);
            this->getButtonMenu()->addChild(toggle);
        }
        this->getButtonMenu()->setContentWidth(count);
        this->getButtonMenu()->setLayout(RowLayout::create());

        this->updateState(nullptr);
        
        return true;
    }
    
    void updateState(CCNode* invoker) override {
        SettingValueNodeV3::updateState(invoker);
        auto shouldEnable = this->getSetting()->shouldEnable();
        for (auto toggle : m_toggles) {
            toggle->toggle(toggle->getTag() == static_cast<int>(this->getValue().m_tab));
            toggle->setEnabled(shouldEnable);
            auto children = toggle->getChildren();
            for (auto children : CCArrayExt<CCNode*>(children)) {
                children->setScale(1);
            }
        }
    }
    void onToggle(CCObject* sender) {
        MyComplexSettingValue Changes = this->getValue();
        Changes.m_tab = sender->getTag();
        this->setValue(Changes, static_cast<CCNode*>(sender));
    }

public:
    static MyCustomSettingNodeV3* create(std::shared_ptr<MyCustomSettingV3> setting, float width) {
        auto ret = new MyCustomSettingNodeV3();
        if (ret && ret->init(setting, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

SettingNodeV3* MyCustomSettingV3::createNode(float width) {
    return MyCustomSettingNodeV3::create(std::static_pointer_cast<MyCustomSettingV3>(shared_from_this()), width);
}

$execute {
    auto ret = Mod::get()->registerCustomSettingType("selectionclicks", &MyCustomSettingV3::parse);
    if (!ret) {
        log::error("Unable to register setting type: {}", ret.unwrapErr());
    }
}