#include <share/ui/imgui/imgui.h>
#include <share/ui/imgui/imgui_internal.h>

namespace ImGui {

bool Switch(const char* sid, const char* label, const char* false_label, const char* true_label,
			bool* value, ImRect* out_rect_only = nullptr);

void AssocViewerItem(const char *assoc);

bool TreeNode(const char *label, bool *ppressed, ImGuiTreeNodeFlags flags = 0);

};