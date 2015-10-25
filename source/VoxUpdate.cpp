#include "VoxGame.h"

#include "utils/Interpolator.h"


// Updating
void VoxGame::Update()
{
	// Update interpolator singleton
	Interpolator::GetInstance()->Update();

	// FPS
	QueryPerformanceCounter(&m_fpsCurrentTicks);
	m_deltaTime = ((float)(m_fpsCurrentTicks.QuadPart - m_fpsPreviousTicks.QuadPart) / (float)m_fpsTicksPerSecond.QuadPart);
	m_fps = 1.0f / m_deltaTime;
	m_fpsPreviousTicks = m_fpsCurrentTicks;

	// Pause the interpolator if animations are paused.
	Interpolator::GetInstance()->SetPaused(m_animationUpdate == false);

	// Animation update
	if (m_animationUpdate)
	{
		// Update the lighting manager
		m_pLightingManager->Update(m_deltaTime);

		// Block particle manager
		m_pBlockParticleManager->Update(m_deltaTime);

		// Update the voxel model
		float animationSpeeds[AnimationSections_NUMSECTIONS] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
		Matrix4x4 worldMatrix;
		m_pVoxelCharacter->Update(m_deltaTime, animationSpeeds);
		m_pVoxelCharacter->UpdateWeaponTrails(m_deltaTime, worldMatrix);
	}

	// Update / Create weapon lights and particle effects
	UpdateWeaponLights(m_deltaTime);
	UpdateWeaponParticleEffects(m_deltaTime);

	// Update controls
	UpdateControls(m_deltaTime);

	// Update lights
	UpdateLights(m_deltaTime);

	// Update the application and window
	m_pVoxApplication->Update(m_deltaTime);
	m_pVoxWindow->Update(m_deltaTime);
}

void VoxGame::UpdateLights(float dt)
{
	m_pRenderer->EditLightPosition(m_defaultLight, m_defaultLightPosition);
}

void VoxGame::UpdateWeaponLights(float dt)
{
	for (int i = 0; i < 2; i++)
	{
		VoxelWeapon* pWeapon = NULL;
		bool isWeaponLoaded = false;
		if (i == 0)  // Right side
		{
			pWeapon = m_pVoxelCharacter->GetRightWeapon();
			isWeaponLoaded = m_pVoxelCharacter->IsRightWeaponLoaded();
		}
		else  // Left side
		{
			pWeapon = m_pVoxelCharacter->GetLeftWeapon();
			isWeaponLoaded = m_pVoxelCharacter->IsLeftWeaponLoaded();
		}

		if (pWeapon != NULL)
		{
			if (isWeaponLoaded)
			{
				for (int i = 0; i < pWeapon->GetNumLights(); i++)
				{
					unsigned int lightId;
					Vector3d lightPos;
					float lightRadius;
					float lightDiffuseMultiplier;
					Colour lightColour;
					bool connectedToSegment;
					pWeapon->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

					if (lightId == -1)
					{
						m_pLightingManager->AddLight(Vector3d(0.0f, 0.0f, 0.0f), 0.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f, 1.0f), &lightId);
						pWeapon->SetLightingId(i, lightId);
					}

					if (connectedToSegment == false)
					{
						// Rotate due to characters forward vector
						//float rotationAngle = acos(Vector3d::DotProduct(Vector3d(0.0f, 0.0f, 1.0f), m_forward));
						//if (m_forward.x < 0.0f)
						//{
						//	rotationAngle = -rotationAngle;
						//}
						//Matrix4x4 rotationMatrix;
						//rotationMatrix.SetRotation(0.0f, rotationAngle, 0.0f);
						//lightPos = rotationMatrix * lightPos;

						//// Translate to position
						//lightPos += m_position;
					}

					float scale = m_pVoxelCharacter->GetCharacterScale();

					m_pLightingManager->UpdateLightPosition(lightId, lightPos);
					m_pLightingManager->UpdateLightRadius(lightId, lightRadius * scale);
					m_pLightingManager->UpdateLightDiffuseMultiplier(lightId, lightDiffuseMultiplier);
					m_pLightingManager->UpdateLightColour(lightId, lightColour);
				}
			}
		}
	}
}

void VoxGame::UpdateWeaponParticleEffects(float dt)
{
	// Create/update
	for (int i = 0; i < 2; i++)
	{
		VoxelWeapon* pWeapon = NULL;
		bool isWeaponLoaded = false;
		if (i == 0)  // Right side
		{
			pWeapon = m_pVoxelCharacter->GetRightWeapon();
			isWeaponLoaded = m_pVoxelCharacter->IsRightWeaponLoaded();
		}
		else  // Left side
		{
			pWeapon = m_pVoxelCharacter->GetLeftWeapon();
			isWeaponLoaded = m_pVoxelCharacter->IsLeftWeaponLoaded();
		}

		if (pWeapon != NULL)
		{
			if (isWeaponLoaded)
			{
				for (int i = 0; i < pWeapon->GetNumParticleEffects(); i++)
				{
					unsigned int particleEffectId;
					Vector3d ParticleEffectPos;
					string effectName;
					bool connectedToSegment;
					pWeapon->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

					if (particleEffectId == -1)
					{
						m_pBlockParticleManager->ImportParticleEffect(effectName, ParticleEffectPos, &particleEffectId);
						pWeapon->SetParticleEffectId(i, particleEffectId);
					}

					if (connectedToSegment == false)
					{
						// Rotate due to characters forward vector
						//float rotationAngle = acos(Vector3d::DotProduct(Vector3d(0.0f, 0.0f, 1.0f), m_forward));
						//if (m_forward.x < 0.0f)
						//{
						//	rotationAngle = -rotationAngle;
						//}
						//Matrix4x4 rotationMatrix;
						//rotationMatrix.SetRotation(0.0f, rotationAngle, 0.0f);
						//ParticleEffectPos = rotationMatrix * ParticleEffectPos;

						//// Translate to position
						//ParticleEffectPos += m_position;
					}

					m_pBlockParticleManager->UpdateParticleEffectPosition(particleEffectId, ParticleEffectPos);
				}
			}
		}
	}
}