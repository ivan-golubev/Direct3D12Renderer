export module GlobalSettings;

namespace awesome::globals {
    export inline constexpr bool IsDebug() { 
#ifdef _DEBUG
        return true;
#else
        return false;
#endif
    }
} // namespace awesome::globals
