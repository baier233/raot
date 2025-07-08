#include "freecam.h"
#include <sdk.h>
static II::Camera* last_main = nullptr;
static II::Camera* our_cam = nullptr;
static II::Vector3 orig_pos;
static II::Quaternion orig_rot;
static std::optional<II::Vector3> current_user_camera_pos;
static std::optional<II::Quaternion> current_user_camera_rot;

static float desiredMoveSpeed = 10.f;
static void cache_main_camera() {
	auto current_cam = II::Camera::GetCurrent();
	auto current_main = II::Camera::GetMain();
	if (current_main)
	{
		last_main = current_main;
		orig_pos = last_main->GetTransform()->GetPosition();
		orig_rot = last_main->GetTransform()->GetRotation();
		if (!current_user_camera_pos.has_value())
		{
			current_user_camera_pos = std::make_optional(orig_pos);
			current_user_camera_rot = std::make_optional(orig_rot);
		}
	}
}
static void setup_freecam() {
	if (freecam::instance().get_toggle())
	{
		if (!last_main)
		{
			freecam::instance().set_toggle(false);
			return;
		}
	}

	if (!our_cam)
	{
		II::GameObject obj;
		II::GameObject::Create(&obj, "UE_Freecam");
		our_cam = obj.AddComponent<II::Camera*>(last_main->GetType());
		if (!our_cam)
		{
			freecam::instance().set_toggle(false);
			return;
		}
		I::DontDestroyOnLoad(our_cam->GetGameObject());
		//modify hide flags
		//...
	}
	last_main->SetEnabled(false);
	our_cam->GetTransform()->SetPosition(current_user_camera_pos.value_or(orig_pos));
	our_cam->GetTransform()->SetRotation(current_user_camera_rot.value_or(orig_rot));

	our_cam->GetGameObject()->SetActive(true);
	//69 our_cam->set
	our_cam->SetEnabled(true);

	current_user_camera_pos = std::nullopt;
	current_user_camera_rot = std::nullopt;

}
void freecam::on_disable()
{

	if (II::Camera::GetCurrent() != our_cam)
	{
		our_cam->Destroy(our_cam);
		our_cam = nullptr;
		return;
	}
	//last_main->SetEnabled(true);
	if (last_main)
	{
		last_main->GetTransform()->SetPosition(orig_pos);
		last_main->GetTransform()->SetRotation(orig_rot);
		//last_main->SetEnabled(true);

	}
	our_cam->GetGameObject()->SetActive(false);
	our_cam->SetEnabled(false);
	our_cam->Destroy(our_cam);
	our_cam = nullptr;
	last_main->SetEnabled(true);

}

void freecam::on_enable()
{
	cache_main_camera();
	setup_freecam();
}

void freecam::on_update()
{
	if (!get_toggle()) return;
	ImGuiIO& io = ImGui::GetIO();
	float moveSpeed = desiredMoveSpeed * io.DeltaTime;
	if (our_cam)
	{
		if (io.KeysDown[ImGuiKey_LeftShift] || io.KeysDown[ImGuiKey_RightShift]) {
			moveSpeed *= 10.0f;
		}
		auto transform = our_cam->GetTransform();

		auto pos = transform->GetPosition();

		if (io.KeysDown[ImGuiKey_W] || io.KeysDown[ImGuiKey_UpArrow])  pos = pos + transform->GetForward() * moveSpeed;
		if (io.KeysDown[ImGuiKey_S] || io.KeysDown[ImGuiKey_DownArrow]) pos = pos - transform->GetForward() * moveSpeed;
		if (io.KeysDown[ImGuiKey_A] || io.KeysDown[ImGuiKey_LeftArrow]) pos = pos - transform->GetRight() * moveSpeed;
		if (io.KeysDown[ImGuiKey_D] || io.KeysDown[ImGuiKey_RightArrow]) pos = pos + transform->GetRight() * moveSpeed;
		if (io.KeysDown[ImGuiKey_Q]) pos = pos - transform->GetUp() * moveSpeed;
		if (io.KeysDown[ImGuiKey_E]) pos = pos + transform->GetUp() * moveSpeed;

		if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
			ImVec2 mouseDelta = io.MouseDelta;
			auto angles = transform->GetLocalEulerAngles();

			angles.y += mouseDelta.x * 0.1f; // Yaw
			angles.x += mouseDelta.y * 0.1f; // Pitch

			transform->SetLocalEulerAngles(II::Vector3{ angles.x,angles.y,0 });
		}

		transform->SetPosition(pos);

	}


}
#include <base/render/gui/gui.h>


freecam::freecam() : instance_module("freecam", category::VISUAL)
{

}
