#ifndef KIT_INPUT_HEADER
#define KIT_INPUT_HEADER

#include "Kit/Export.hpp"
#include "Kit/OpenGL.hpp"

#include <array>
#include <memory>

namespace kit{

enum KITAPI MouseButton{
  Mouse1 = 0,
  Mouse2 = 1,
  Mouse3 = 2,
  Mouse4 = 3,
  Mouse5 = 4,
  Mouse6 = 5,
  Mouse7 = 6,
  Mouse8 = 7,
  MouseLast = Mouse8,
  LeftMouse = Mouse1,
  RightMouse = Mouse2,
  MiddleMouse= Mouse3
};

enum KITAPI Key{
        Unknown = -1,
        Space = 32,
        Apostrophe = 39,
        Comma = 44,
        Minus = 45,
        Period = 46,
        Slash = 47,
        Num0 = 48,
        Num1 = 49,
        Num2 = 50,
        Num3 = 51,
        Num4 = 52,
        Num5 = 53,
        Num6 = 54,
        Num7 = 55,
        Num8 = 56,
        Num9 = 57,
        Semicolon = 59,
        Equal = 61,
        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        LeftBracket = 91,
        Backslash = 92,
        RightBracket = 93,
        GraveAccent = 96,
        World1 = 161,
        World2 = 162,
        Escape = 256,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Insert = 260,
        Delete = 261,
        Right = 262,
        Left = 263,
        Down = 264,
        Up = 265,
        Page_up = 266,
        Page_down = 267,
        Home = 268,
        End = 269,
        CapsLock = 280,
        ScrollLock = 281,
        NumLock = 282,
        PrintScreen = 283,
        pause = 284,
        F1 = 290,
        F2 = 291,
        F3 = 292,
        F4 = 293,
        F5 = 294,
        F6 = 295,
        F7 = 296,
        F8 = 297,
        F9 = 298,
        F10 = 299,
        F11 = 300,
        F12 = 301,
        F13 = 302,
        F14 = 303,
        F15 = 304,
        F16 = 305,
        F17 = 306,
        F18 = 307,
        F19 = 308,
        F20 = 309,
        F21 = 310,
        F22 = 311,
        F23 = 312,
        F24 = 313,
        F25 = 314,
        Kp_0 = 320,
        Kp_1 = 321,
        Kp_2 = 322,
        Kp_3 = 323,
        Kp_4 = 324,
        Kp_5 = 325,
        Kp_6 = 326,
        Kp_7 = 327,
        Kp_8 = 328,
        Kp_9 = 329,
        KpDecimal = 330,
        KpDivide = 331,
        KpMultiply = 332,
        KpSubtract = 333,
        KpAdd = 334,
        KpEnter = 335,
        KpEqual = 336,
        LeftShift = 340,
        LeftControl = 341,
        LeftAlt = 342,
        LeftSuper = 343,
        RightShift = 344,
        RightControl = 345,
        RightAlt = 346,
        RightSuper = 347,
        Menu = 348,
        Last = Menu
};

enum KITAPI ModifierKey{
        ModShift = 0x0001,
        ModControl = 0x0002,
        ModAlt = 0x0004,
        ModSuper = 0x0008
};

#ifdef KIT_EXPERIMENTAL_CONTROLLER
class KITAPI Controller
{
  public:
    typedef std::shared_ptr<Controller> Ptr;

    /// Polls for newly connected controllers
    static void refreshControllers();

    /// Returns the current cache of connected controllers
    static std::array<Controller, 16> & getControllers();

    /// Refreshes the states for the current controller
    void refreshStates();

    /// True if the controller is connected, false otherwise
    bool isConnected();

    /// Unique identifier
    uint32_t const & getId();

    /// Non-unique controller name
    std::string const & getName();

    /// Check if button is down
    bool isButtonDown(uint32_t button_id);

    /// Get axis value
    float getAxis(uint32_t axis_id);

    uint32_t getNumAxis();

    Controller();
  private:


    kit::GLFWSingleton m_glfwSingleton;

    static std::array<Controller, 16> m_controllerCache;

    uint32_t m_id;
    std::string m_name;
    bool m_connected;

    std::vector<float> m_axisStates;
    std::vector<bool> m_buttonStates;

};
#endif
}

#endif // KIT_INPUT_HEADER
