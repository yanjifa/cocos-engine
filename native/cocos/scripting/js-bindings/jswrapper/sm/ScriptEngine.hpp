#pragma once

#include "../config.hpp"

#ifdef SCRIPT_ENGINE_SM

#include "Base.h"

#include <chrono>

namespace se {

    class Object;
    class Class;
    class Value;

    extern Class* __jsb_CCPrivateData_class;
    
    class AutoHandleScope
    {
    public:
        AutoHandleScope();
        ~AutoHandleScope();
    };
    
    class ScriptEngine final
    {
    private:
        ScriptEngine();
        ~ScriptEngine();

    public:
        static ScriptEngine* getInstance();
        static void destroyInstance();

        Object* getGlobalObject();

        typedef bool (*RegisterCallback)(Object*);
        void addRegisterCallback(RegisterCallback cb);
        bool start();

        bool init();
        void cleanup();
        /**
         *  @brief Adds a hook function before initializing script engine.
         *  @param[in] hook A hook function to be invoked before initializing script engine.
         *  @note Multiple hook functions could be added, they will be invoked by the order of adding.
         */
        void addBeforeInitHook(const std::function<void()>& hook);

        /**
         *  @brief Adds a hook function after initializing script engine.
         *  @param[in] hook A hook function to be invoked before initializing script engine.
         *  @note Multiple hook functions could be added, they will be invoked by the order of adding.
         */
        void addAfterInitHook(const std::function<void()>& hook);

        void addBeforeCleanupHook(const std::function<void()>& hook);
        void addAfterCleanupHook(const std::function<void()>& hook);

        bool evalString(const char* string, ssize_t length = -1, Value* ret = nullptr, const char* fileName = nullptr);

        class FileOperationDelegate
        {
        public:
            FileOperationDelegate()
            : onGetDataFromFile(nullptr)
            , onGetStringFromFile(nullptr)
            , onCheckFileExist(nullptr)
            , onGetFullPath(nullptr)
            {}

            bool isValid() {
                return onGetDataFromFile != nullptr
                    && onGetStringFromFile != nullptr
                    && onCheckFileExist != nullptr
                    && onGetFullPath != nullptr; }

            // path, buffer, buffer size
            std::function<void(const std::string&, const uint8_t**, size_t*)> onGetDataFromFile;
            // path, return file string content.
            std::function<std::string(const std::string&)> onGetStringFromFile;
            // path
            std::function<bool(const std::string&)> onCheckFileExist;
            // path, return full path
            std::function<std::string(const std::string&)> onGetFullPath;
        };

        void setFileOperationDelegate(const FileOperationDelegate& delegate);

        bool runScript(const std::string& fullPath, Value* ret = nullptr);

        JSContext* _getContext() { return _cx; }

        bool isGarbageCollecting();
        void _setGarbageCollecting(bool isGarbageCollecting);
        void garbageCollect() { JS_GC( _cx );  }

        bool isValid() { return _isValid; }
        bool isInCleanup() { return _isInCleanup; }

        void clearException();

        const std::chrono::steady_clock::time_point& getStartTime() const { return _startTime; }

        void _retainScriptObject(void* owner, void* target);
        void _releaseScriptObject(void* owner, void* target);

        enum class NodeEventType
        {
            ENTER,
            EXIT,
            ENTER_TRANSITION_DID_FINISH,
            EXIT_TRANSITION_DID_START,
            CLEANUP
        };
        bool _onReceiveNodeEvent(void* node, NodeEventType type);

        using NodeEventListener = bool(*)(void*, NodeEventType);
        bool _setNodeEventListener(NodeEventListener listener);

    private:
        static void myWeakPointerCompartmentCallback(JSContext* cx, JSCompartment* comp, void* data);
        static void myWeakPointerZoneGroupCallback(JSContext* cx, void* data);

        bool getScript(const std::string& path, JS::MutableHandleScript script);
        bool compileScript(const std::string& path, JS::MutableHandleScript script);

        JSContext* _cx;
        JSCompartment* _oldCompartment;

        Object* _globalObj;

        bool _isGarbageCollecting;
        bool _isValid;
        bool _isInCleanup;
        NodeEventListener _nodeEventListener;

        FileOperationDelegate _fileOperationDelegate;

        std::vector<RegisterCallback> _registerCallbackArray;
        std::chrono::steady_clock::time_point _startTime;

        std::vector<std::function<void()>> _beforeInitHookArray;
        std::vector<std::function<void()>> _afterInitHookArray;

        std::vector<std::function<void()>> _beforeCleanupHookArray;
        std::vector<std::function<void()>> _afterCleanupHookArray;

        // name ~> JSScript map
        std::unordered_map<std::string, JS::PersistentRootedScript*> _filenameScriptMap;
    };

 } // namespace se {

#endif // SCRIPT_ENGINE_SM


