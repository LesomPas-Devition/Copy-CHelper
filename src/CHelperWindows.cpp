//
// Created by Yancey on 2024/2/26.
//

#include "CHelperWindows.h"
#include "chelper/Core.h"
#include "chelper/util/TokenUtil.h"
#include <commctrl.h>

static size_t ID_INPUT = 1;
static size_t ID_DESCRIPTION = 2;
static size_t ID_LIST_VIEW = 3;

static TCHAR szWindowClass[] = "CHelper";
static TCHAR szTitle[] = "CHelper";

static std::shared_ptr<CHelper::Core> core;

/**
 * @param hInstance 应用程序的当前实例的句柄
 * @param hPrevInstance 应用程序上一个实例的句柄
 * @param lpCmdLine 应用程序的命令行，不包括程序名称
 * @param nCmdShow 控制窗口的显示方式
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    core = CHelper::Core::create(R"(D:\CLion\project\CHelper\resources)");
    if (core == nullptr) {
        exit(-1);
    }
    //窗口数据
    WNDCLASSEX wcex;
    wcex.cbWndExtra = 0;
    wcex.cbClsExtra = 0;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    //注册窗口
    if (!RegisterClassEx(&wcex)) {
        MessageBox(nullptr, "Call to RegisterClassEx failed!", "CHelper", 0);
        return 1;
    }
    //创建窗口
    HWND hWnd = CreateWindowEx(
            WS_EX_OVERLAPPEDWINDOW,
            szWindowClass,
            szTitle,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            500, 500,
            nullptr, nullptr,
            hInstance, nullptr
    );
    if (hWnd == nullptr) {
        MessageBox(nullptr, "Call to CreateWindow failed!", "CHelper", 0);
        return 1;
    }
    //显示并更新窗口
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    //循环
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int) msg.wParam;
}

static HWND hWndInput, hWndDescription, hWndListBox;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hWndInput = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                     0, 0, 0, 0, hWnd, (HMENU) ID_INPUT, nullptr, nullptr);
            hWndDescription = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                           0, 0, 0, 0, hWnd, (HMENU) ID_DESCRIPTION, nullptr, nullptr);
            hWndListBox = CreateWindow(WC_LISTBOX, "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                                       0, 0, 0, 0, hWnd, (HMENU) ID_LIST_VIEW, nullptr, nullptr);
            onTextChanged("");
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_INPUT && HIWORD(wParam) == EN_CHANGE) {
                CHelper::Profile::push("get command from input");
                int length = GetWindowTextLength(hWndInput);
                auto *buffer = new char[length + 1];
                GetWindowText(hWndInput, buffer, length + 1);
                UINT codePage = GetACP();
                int wideCharLength = MultiByteToWideChar(codePage, 0, buffer, -1, nullptr, 0);
                auto *wideCharBuffer = new wchar_t[wideCharLength + 1];
                MultiByteToWideChar(codePage, 0, buffer, -1, wideCharBuffer, wideCharLength + 1);
                int utf8Length = WideCharToMultiByte(CP_UTF8, 0, wideCharBuffer, -1, nullptr, 0, nullptr, nullptr);
                char *utf8Buffer = new char[utf8Length + 1];
                WideCharToMultiByte(CP_UTF8, 0, wideCharBuffer, -1, utf8Buffer, utf8Length + 1, nullptr, nullptr);
                CHelper::Profile::pop();
                onTextChanged(utf8Buffer);
                delete[] buffer;
                delete[] wideCharBuffer;
                delete[] utf8Buffer;
            }
            break;
        case WM_SIZE:
            MoveWindow(hWndInput, 10, HIWORD(lParam) - 30, LOWORD(lParam) - 20, 20, TRUE);
            MoveWindow(hWndDescription, 10, 10, LOWORD(lParam) - 20, 20, TRUE);
            MoveWindow(hWndListBox, 10, 40, LOWORD(lParam) - 20, HIWORD(lParam) - 70, TRUE);
            break;
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void onTextChanged(const std::string &command) {
    try {
        clock_t start1, end1, start2, end2;
        CHelper::Profile::push(CHelper::ColorStringBuilder()
                                       .red("parsing command: ")
                                       .purple(command)
                                       .build());
        start1 = clock();
        core->onTextChanged(command, command.length());
        auto description = core->getDescription();
        auto errorReasons = core->getErrorReasons();
        auto suggestions = core->getSuggestions();
        auto structure = core->getStructure();
        end1 = clock();
        start2 = clock();
        CHelper::Profile::next("get description text view content");
        {
            CHelper::Profile::next("update description text view");
            int len = MultiByteToWideChar(CP_UTF8, 0, description.c_str(), -1, nullptr, 0);
            auto *wstr = new wchar_t[len + 1];
            MultiByteToWideChar(CP_UTF8, 0, description.c_str(), -1, wstr, len);
            SetWindowTextW(hWndDescription, wstr);
            delete[] wstr;
        }
        CHelper::Profile::next("update suggestion list view");
        SendMessage(hWndListBox, LB_RESETCONTENT, 0, 0);
        //由于添加全部结果非常耗时，这里只保留前30个
        int i = 0;
        for (const auto &suggestion: suggestions) {
            if (++i > 30) {
                break;
            }
            auto content = std::string(suggestion.content->name).append(" - ")
                    .append(suggestion.content->description.value_or(""));
            int len = MultiByteToWideChar(CP_UTF8, 0, content.c_str(), -1, nullptr, 0);
            auto *wstr = new wchar_t[len];
            MultiByteToWideChar(CP_UTF8, 0, content.c_str(), -1, wstr, len);
            SendMessageW(hWndListBox, LB_ADDSTRING, 0, (LPARAM) wstr);
            delete[] wstr;
        }
        CHelper::Profile::pop();
        end2 = clock();
        CHELPER_INFO(CHelper::ColorStringBuilder()
                             .green("update successfully in ")
                             .purple(std::to_string(end2 - start1) + "ms")
                             .green(" : parse in ")
                             .purple(std::to_string(end1 - start1) + "ms")
                             .green(", show in ")
                             .purple(std::to_string(end2 - start2) + "ms")
                             .green(", command is ")
                             .purple(command.size() <= 100 ? command : command.substr(0, 100) + "...")
                             .build());
//        std::cout << core->getAstNode().toOptimizedJson() << std::endl;
//        std::cout << core->getAstNode().toBestJson() << std::endl;
        CHELPER_INFO("structure: " + structure);
        CHELPER_INFO("description: " + description);
        if (errorReasons.empty()) {
            CHELPER_INFO("no error");
        } else if (errorReasons.size() == 1) {
            const auto &errorReason = errorReasons[0];
            CHELPER_INFO(CHelper::ColorStringBuilder()
                                 .normal("error reason: ")
                                 .red(command.substr(errorReason->start,
                                                     errorReason->end - errorReason->start) + " ")
                                 .blue(errorReason->errorReason)
                                 .build());
            CHELPER_INFO(CHelper::ColorStringBuilder()
                                 .normal(command.substr(0, errorReason->start))
                                 .red(errorReason->start == errorReason->end ? "~" :
                                      command.substr(errorReason->start,
                                                     errorReason->end - errorReason->start))
                                 .normal(command.substr(errorReason->end))
                                 .build());
        } else {
            CHELPER_INFO("error reasons:");
            int i2 = 0;
            for (const auto &errorReason: errorReasons) {
                CHELPER_INFO(CHelper::ColorStringBuilder()
                                     .normal(std::to_string(++i2) + ". ")
                                     .red(command.substr(errorReason->start,
                                                         errorReason->end - errorReason->start) + " ")
                                     .blue(errorReason->errorReason)
                                     .build());
                CHELPER_INFO(CHelper::ColorStringBuilder()
                                     .normal(command.substr(0, errorReason->start))
                                     .red(errorReason->start == errorReason->end ? "~" :
                                          command.substr(errorReason->start,
                                                         errorReason->end - errorReason->start))
                                     .normal(command.substr(errorReason->end))
                                     .build());
            }
        }
        if (suggestions.empty()) {
            CHELPER_INFO("no suggestion");
        } else {
            CHELPER_INFO("suggestions: ");
            int j = 0;
            for (const auto &item: suggestions) {
                if (j == 30) {
                    CHELPER_INFO("...");
                    break;
                }
                CHELPER_INFO(CHelper::ColorStringBuilder()
                                     .normal(std::to_string(++j) + ". ")
                                     .green(item.content->name + " ")
                                     .blue(item.content->description.value_or(""))
                                     .build());
                CHELPER_INFO(CHelper::ColorStringBuilder()
                                     .normal(command.substr(0, item.start))
                                     .green(item.content->name)
                                     .normal(command.substr(item.end))
                                     .build());
            }
        }
        std::cout << std::endl;
    } catch (const std::exception &e) {
        CHELPER_INFO(CHelper::ColorStringBuilder().red("parse failed").build());
        CHelper::Exception::printStackTrace(e);
        CHelper::Profile::clear();
    }
}