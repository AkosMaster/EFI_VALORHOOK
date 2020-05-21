#include "DirectXWindow.h"

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "vector3.h"

#define OFFSET_WORLD 0x627ced8
#define PersistentLevel 0x38
#define OwningGameInstance 0x190
#define LocalPlayers 0x40
#define PlayerController 0x38
#define AcknowledgedPawn 0x430
#define ControlRotation 0x418
#define RootComponent 0x238
#define Mesh 0x408
#define DamageHandler 0xA50
#define CachedLife 0x190
#define TeamComponent 0x5B0
#define TeamIndex 0x118
#define PlayerName 0x3A8
#define StaticMesh 0x528
#define PlayerCameraManager 0x448
#define RelativeLocation 0x184
#define RelativeRotation 0x190
#define PlayerState 0x3C8

#ifndef GAMEMATH
#define GAMEMATH

inline D3DXMATRIX ToMatrix(Vector3 Rotation, Vector3 origin = Vector3(0, 0, 0))
{

	float Pitch = (Rotation.x * float(M_PI) / 180.f);
	float Yaw = (Rotation.y * float(M_PI) / 180.f);
	float Roll = (Rotation.z * float(M_PI) / 180.f);

	float SP = sinf(Pitch);
	float CP = cosf(Pitch);
	float SY = sinf(Yaw);
	float CY = cosf(Yaw);
	float SR = sinf(Roll);
	float CR = cosf(Roll);

	D3DXMATRIX Matrix;
	Matrix._11 = CP * CY;
	Matrix._12 = CP * SY;
	Matrix._13 = SP;
	Matrix._14 = 0.f;

	Matrix._21 = SR * SP * CY - CR * SY;
	Matrix._22 = SR * SP * SY + CR * CY;
	Matrix._23 = -SR * CP;
	Matrix._24 = 0.f;

	Matrix._31 = -(CR * SP * CY + SR * SY);
	Matrix._32 = CY * SR - CR * SP * SY;
	Matrix._33 = CR * CP;
	Matrix._34 = 0.f;

	Matrix._41 = origin.x;
	Matrix._42 = origin.y;
	Matrix._43 = origin.z;
	Matrix._44 = 1.f;

	return Matrix;
}

inline Vector3 WorldToScreen(Vector3 world_location, Vector3 position, Vector3 rotation, float fov)
{
	Vector3 screen_location = Vector3(0, 0, 0);

	D3DMATRIX tempMatrix = ToMatrix(rotation);

	Vector3 vAxisX, vAxisY, vAxisZ;

	vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = world_location - position;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	float FovAngle = fov;
	float ScreenCenterX = ScreenInfo::Width / 2.0f;
	float ScreenCenterY = ScreenInfo::Height / 2.0f;

	screen_location.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	screen_location.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;

	return screen_location;
}

inline Vector2 rotatePointAroundPivot(float cx, float cy, float angle, Vector2 p)
{
	float s = sin(angle);
	float c = cos(angle);

	// translate point back to origin:
	p.x -= cx;
	p.y -= cy;

	// rotate point
	float xnew = p.x * c - p.y * s;
	float ynew = p.x * s + p.y * c;

	// translate point back:
	p.x = xnew + cx;
	p.y = ynew + cy;
	return p;
}

inline double deg2rad(double degrees) {
	return degrees * 4.0 * atan(1.0) / 180.0;
}

#endif