
#include "ai-editor-view.h"

void Example::OnStart()
{
    ed::Config config;
    config.SettingsFile = "Widgets.json";
    m_Context = ed::CreateEditor(&config);

    ed::SetCurrentEditor(m_Context);

    Node* node;
    node = SpawnBranchNode();      ed::SetNodePosition(node->ID, ImVec2(0, 0));
    node = SpawnBranchNode();      ed::SetNodePosition(node->ID, ImVec2(100, 100));

    BuildNodes();
}

void Example::OnFrame()
{
    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;


    if (ImGui::Begin("EditorView", NULL, window_flags))
    {

        ed::SetCurrentEditor(m_Context);
        ed::Begin("My Editor", ImVec2(0.0, 0.0f));


        for (auto& node : m_Nodes)
        {
            ed::BeginNode(node.ID);
            ImGui::Text("Node A");
            for (auto& input : node.Inputs)
            {
                ed::BeginPin(input.ID, ed::PinKind::Input);
                    ImGui::Text("-> In");
                ed::EndPin();
            }

            for (auto& output : node.Outputs)
            {
                ed::BeginPin(output.ID, ed::PinKind::Output);
                    ImGui::Text("Out ->");
                ed::EndPin();
            }

            ed::EndNode();
        }

        ed::End();
        ed::SetCurrentEditor(nullptr);


    }

    ImGui::End();



}
