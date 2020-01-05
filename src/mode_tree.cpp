#include "zep/mode_tree.h"
#include "zep/filesystem.h"
#include "zep/tab_window.h"
#include "zep/window.h"
#include "zep/editor.h"

#include "zep/mcommon/logger.h"
#include "zep/mcommon/threadutils.h"

namespace Zep
{

ZepMode_Tree::ZepMode_Tree(ZepEditor& editor, ZepWindow& launchWindow, ZepWindow& window)
    : ZepMode(editor),
    m_launchWindow(launchWindow),
    m_window(window)
{
}

ZepMode_Tree::~ZepMode_Tree()
{
}

void ZepMode_Tree::Close()
{
    GetEditor().RemoveBuffer(&m_window.GetBuffer());
}

void ZepMode_Tree::AddKeyPress(uint32_t key, uint32_t modifiers)
{
    auto pGlobalMode = GetEditor().GetGlobalMode();

    GetEditor().ResetLastEditTimer();

    if (key == 'r' && modifiers == ModifierKey::Ctrl)
    {
        Close();
        return;
    }

    // If not in insert mode, then let the normal mode do its thing
    if (pGlobalMode->GetEditorMode() != Zep::EditorMode::Insert)
    {
        pGlobalMode->AddKeyPress(key, modifiers);

        m_currentMode = pGlobalMode->GetEditorMode();
        if (pGlobalMode->GetEditorMode() == Zep::EditorMode::Insert)
        {
            // Set the cursor to the end of the buffer while inserting text
            m_window.SetBufferCursor(MaxCursorMove);
            m_window.SetCursorType(CursorType::Insert);
        }
        return;
    }
  
    // Set the cursor to the end of the buffer while inserting text
    m_window.SetBufferCursor(MaxCursorMove);
    m_window.SetCursorType(CursorType::Insert);

    (void)modifiers;
    if (key == ExtKeys::ESCAPE)
    {
        // Escape back to the default normal mode
        GetEditor().GetGlobalMode()->Begin();
        GetEditor().GetGlobalMode()->SetEditorMode(EditorMode::Normal);
        m_currentMode = Zep::EditorMode::Normal;
        return;
    } 
    else if (key == ExtKeys::RETURN)
    {
        auto& buffer = m_window.GetBuffer();
        std::string str = std::string(buffer.GetText().begin() + m_startLocation, buffer.GetText().end());
        buffer.Insert(buffer.EndLocation(), "\n");

        /*
        auto stripLineStarts = [](std::string& str)
        {
            bool newline = true;
            int pos = 0;
            while (pos < str.size())
            {
                if (str[pos] == '\n')
                    newline = true;
                else if (newline)
                {
                    if (str.find(PromptString, pos) == pos)
                    {
                        str.erase(pos, PromptString.length());
                        continue;
                    }
                    else if (str.find(ContinuationString, pos) == pos)
                    {
                        str.erase(pos, ContinuationString.length());
                        continue;
                    }
                    newline = false;
                }
                pos++;
            }
        };
        stripLineStarts(str);

        std::string ret;
        if (m_pRepl)
        {
            int indent = 0;
            bool complete = m_pRepl->fnIsFormComplete ? m_pRepl->fnIsFormComplete(str, indent) : true;
            if (!complete)
            {
                // If the indent is < 0, we completed too much of the expression, so don't let the user hit return until they 
                // fix it.  Example in lisp: (+ 2 2))  This expression has 'too many' close brackets.
                if (indent < 0)
                {
                    buffer.Delete(buffer.EndLocation() - 1, buffer.EndLocation());
                    m_window.SetBufferCursor(MaxCursorMove);
                    return;
                }
                   
                // New line continuation symbol
                buffer.Insert(buffer.EndLocation(), ContinuationString);

                // Indent by how far the repl suggests
                if (indent > 0)
                {
                    for (int i = 0; i < indent; i++)
                    {
                        buffer.Insert(buffer.EndLocation(), " ");
                    }
                }
                m_window.SetBufferCursor(MaxCursorMove);
                return;
            }
            ret = m_pRepl->fnParser(str);
        }
        else
        {
            ret = str;
        }

        if (!ret.empty() && ret[0] != 0)
        {
            ret.push_back('\n');
            buffer.Insert(buffer.EndLocation(), ret);
        }
        */

        BeginInput();
        return;
    }
    else if (key == ExtKeys::BACKSPACE)
    {
        auto cursor = m_window.GetBufferCursor() - 1;
        if (cursor >= m_startLocation)
        {
            m_window.GetBuffer().Delete(m_window.GetBufferCursor() - 1, m_window.GetBufferCursor());
        }
    }
    else
    {
        char c[2];
        c[0] = (char)key;
        c[1] = 0;
        m_window.GetBuffer().Insert(m_window.GetBufferCursor(), std::string(c));
    }

    // Ensure cursor is at buffer end
    m_window.SetBufferCursor(MaxCursorMove);
}

void ZepMode_Tree::BeginInput()
{
    // Input arrows
    //auto& buffer = m_window.GetBuffer();
    //buffer.Insert(buffer.EndLocation(), PromptString);

    m_window.SetBufferCursor(MaxCursorMove);
    m_startLocation = m_window.GetBufferCursor();
}

void ZepMode_Tree::Begin()
{
    // Default insert mode
    GetEditor().GetGlobalMode()->Begin();
    GetEditor().GetGlobalMode()->SetEditorMode(EditorMode::Insert);
    m_currentMode = EditorMode::Insert;
    m_window.SetCursorType(CursorType::Insert);

    GetEditor().SetCommandText("");
    
    BeginInput();
}

void ZepMode_Tree::Notify(std::shared_ptr<ZepMessage> message)
{
    ZepMode::Notify(message);
}

} // namespace Zep
