#include "memory.h"
#include "vector3.h"
#include <iostream>
#include <thread>

namespace offsets
{
	// client offsets
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDEA98C;
	constexpr ::std::ptrdiff_t dwEntityList = 0x4DFFF7C;

	// entity offsets
	constexpr ::std::ptrdiff_t m_iTeamNum = 0xF4;
	constexpr ::std::ptrdiff_t m_lifeState = 0x25F;
	constexpr ::std::ptrdiff_t m_dwBoneMatrix = 0x26A8;
	constexpr ::std::ptrdiff_t m_bDormant = 0xED;
	constexpr ::std::ptrdiff_t m_vecOrigin = 0x138;
	constexpr ::std::ptrdiff_t m_vecViewOffset = 0x108;
	constexpr ::std::ptrdiff_t m_aimPunchAngle = 0x303C;
	constexpr ::std::ptrdiff_t m_bSpottedByMask = 0x980;

	// engine offsets
	constexpr ::std::ptrdiff_t dwClientState = 0x59F19C;
	constexpr ::std::ptrdiff_t dwClientState_ViewAngles = 0x4D90;
	constexpr ::std::ptrdiff_t dwClientState_GetLocalPlayer = 0x180;
}

constexpr Vector3 calcAngle(
	const Vector3& localPos,
	const Vector3& enemyPos,
	const Vector3& viewAngles) noexcept
{
	return ((enemyPos - localPos).toAngle() - viewAngles);
}

int main()
{
	/*
		1. looping over entities
		2. checking if entity is a valid target
		3. getting their head's position
		4. getting the angle between entities and local player
		5. setting FOV and view angle
	*/

	Memory mem{ "csgo.exe" };
	std::cout << "[+] Process ID -> " << mem.getProcessId() << '\n';

	const auto client = mem.getModuleAddress("client.dll");
	std::cout << "[+] Client Dll -> 0x" << std::hex << client << std::dec << '\n';

	const auto engine = mem.getModuleAddress("engine.dll");
	std::cout << "[+] Client Dll -> 0x" << std::hex << engine << std::dec << '\n';

	std::cout << "--------------------------------------------" << '\n';
	std::cout << "~ Starting The Cheat... ~" << '\n';

	constexpr int headBoneIndex = 8;
	constexpr float smoothingFactor = 3.f;

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (!GetAsyncKeyState(VK_RBUTTON))
			continue;

		const auto localPlayer = mem.Read<uintptr_t>(client + offsets::dwLocalPlayer);
		const auto playerTeam = mem.Read<int32_t>(localPlayer + offsets::m_iTeamNum);

		const auto playerEyePos = mem.Read<Vector3>(localPlayer + offsets::m_vecOrigin)
			+ mem.Read<Vector3>(localPlayer + offsets::m_vecViewOffset);

		const auto clientState = mem.Read<uintptr_t>(engine + offsets::dwClientState);
		const auto localPlayerId = mem.Read<int32_t>(clientState + offsets::dwClientState_GetLocalPlayer);

		const auto viewAngles = mem.Read<Vector3>(clientState + offsets::dwClientState_ViewAngles);
		const auto aimPunch = mem.Read<Vector3>(localPlayer + offsets::m_aimPunchAngle) * 2;

		float opFov = 5.f;
		Vector3 opAngle{ };

		for (int i = 1; i <= 32; ++i)
		{
			const auto entity = mem.Read<uintptr_t>(client + offsets::dwEntityList + i * 0x10);

			if (mem.Read<int32_t>(entity + offsets::m_iTeamNum) == playerTeam)
				continue;

			if (mem.Read<bool>(entity + offsets::m_bDormant))
				continue;

			if (mem.Read<int32_t>(entity + offsets::m_lifeState))
				continue;

			if (mem.Read<int32_t>(entity + offsets::m_bSpottedByMask) & (1 << localPlayerId))
			{
				const auto boneMatrix = mem.Read<uintptr_t>(entity + offsets::m_dwBoneMatrix);

				// getting the entity's head position
				Vector3 entityHeadPos{
					mem.Read<float>(boneMatrix + 0x30 * 8 + 0x0C),
					mem.Read<float>(boneMatrix + 0x30 * 8 + 0x1C),
					mem.Read<float>(boneMatrix + 0x30 * 8 + 0x2C)
				};

				// calculating the angle and FOV
				const auto angle = calcAngle(playerEyePos, entityHeadPos, viewAngles + aimPunch);
				const auto fov = std::hypot(angle.x, angle.y);

				if (fov < opFov)
				{
					opFov = fov;
					opAngle = angle;
				}
			}
		}

		if (!opAngle.isZero())
		{
			mem.Write<Vector3>(clientState + offsets::dwClientState_ViewAngles, viewAngles + opAngle / smoothingFactor);
		}
	}

	return 0;
}
