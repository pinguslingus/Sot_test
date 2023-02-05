#pragma once
#include <Windows.h>
#include <UE4/UE4.h>
#include <string>

struct UFunction;
class UClass;

inline void ProcessEvent(void* obj, UFunction* function, void* parms)
{
	auto vtable = *reinterpret_cast<void***>(obj);
	reinterpret_cast<void(*)(void*, UFunction*, void*)>(vtable[55])(obj, function, parms);
}

template<typename Fn>
inline Fn GetVFunction(const void* instance, std::size_t index)
{
	auto vtable = *reinterpret_cast<const void***>(const_cast<void*>(instance));
	return reinterpret_cast<Fn>(vtable[index]);
}

struct FNameEntry
{
	uint32_t Index;
	uint32_t pad;
	FNameEntry* HashNext;
	char AnsiName[1024];

	const int GetIndex() const { return Index >> 1; }
	const char* GetAnsiName() const { return AnsiName; }
};

class TNameEntryArray
{
public:

	bool IsValidIndex(uint32_t index) const { return index < NumElements; }

	FNameEntry const* GetById(uint32_t index) const { return *GetItemPtr(index); }

	FNameEntry const* const* GetItemPtr(uint32_t Index) const {
		const auto ChunkIndex = Index / 16384;
		const auto WithinChunkIndex = Index % 16384;
		const auto Chunk = Chunks[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

	FNameEntry** Chunks[128];
	uint32_t NumElements = 0;
	uint32_t NumChunks = 0;
};

// @abrn
struct FTextData
{
public:
	wchar_t* Text; //0x0000 (0x08)
};

struct FText
{
public:
	struct FTextData* TextData; //0x0000 (0x08)
};

class FString : public TArray<wchar_t>
{
public:
	inline FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? static_cast<int32_t>(std::wcslen(other)) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	int multi(char* name, int size) const
	{
		return WideCharToMultiByte(CP_UTF8, 0, Data, Count, name, size, nullptr, nullptr) - 1;
	}

	inline const wchar_t* wide() const
	{
		return Data;
	}

	inline const wchar_t* c_str() const
	{
		if (Data)
			return Data;
		return L"";
	}
};

struct FName
{
	int ComparisonIndex = 0;
	int Number = 0;

	static inline TNameEntryArray* GNames = nullptr;

	static const char* GetNameByIdFast(int Id) {
		auto NameEntry = GNames->GetById(Id);
		if (!NameEntry) return nullptr;
		return NameEntry->GetAnsiName();
	}

	static std::string GetNameById(int Id) {
		auto NameEntry = GNames->GetById(Id);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	}

	const char* GetNameFast() const {
		auto NameEntry = GNames->GetById(ComparisonIndex);
		if (!NameEntry) return nullptr;
		return NameEntry->GetAnsiName();
	}

	const std::string GetName() const {
		auto NameEntry = GNames->GetById(ComparisonIndex);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	};

	inline bool operator==(const FName& other) const {
		return ComparisonIndex == other.ComparisonIndex;
	};

	FName() {}

	FName(const char* find) {
		for (auto i = 6000u; i < GNames->NumElements; i++)
		{
			auto name = GetNameByIdFast(i);
			if (!name) continue;
			if (strcmp(name, find) == 0) {
				ComparisonIndex = i;
				return;
			};
		}
	}
};

struct FUObjectItem
{
	class UObject* Object;
	int Flags;
	int ClusterIndex;
	int SerialNumber;
	int pad;
};

struct TUObjectArray
{
	FUObjectItem* Objects;
	int MaxElements;
	int NumElements;

	class UObject* GetByIndex(int index) { return Objects[index].Object; }
};

// Class CoreUObject.Object
// Size: 0x28 (Inherited: 0x00)
struct UObject {
	UObject(UObject* addr) { *this = addr; }
	static inline TUObjectArray* GObjects = nullptr;
	void* Vtable; // 0x0
	int ObjectFlags; // 0x8
	int InternalIndex; // 0xC
	UClass* Class; // 0x10
	FName Name; // 0x18
	UObject* Outer; // 0x20

	std::string GetName() const;
	const char* GetNameFast() const;
	std::string GetFullName() const;
	bool IsA(UClass* cmp) const;

	template<typename T>
	static T* FindObject(const std::string& name)
	{
		for (int i = 0; i < GObjects->NumElements; ++i)
		{
			auto object = GObjects->GetByIndex(i);

			if (object == nullptr)
			{
				continue;
			}

			if (object->GetFullName() == name)
			{
				return static_cast<T*>(object);
			}
		}
		return nullptr;
	}

	static UClass* FindClass(const std::string& name)
	{
		return FindObject<UClass>(name);
	}
	
};

struct FVector_NetQuantize : public FVector
{

};
struct FHitResult
{
	unsigned char                                      bBlockingHit : 1;                                         // 0x0000(0x0001)
	unsigned char                                      bStartPenetrating : 1;                                    // 0x0000(0x0001)
	unsigned char                                      UnknownData00[0x3];                                       // 0x0001(0x0003) MISSED OFFSET
	float                                              Time;                                                     // 0x0004(0x0004) (ZeroConstructor, IsPlainOldData)
	float                                              Distance;                                                 // 0x0008(0x0004) (ZeroConstructor, IsPlainOldData)
	struct FVector_NetQuantize                         ImpactPoint;                                              // 0x0018(0x000C)
	char pad_5894[0x48];
	float                                              PenetrationDepth;                                         // 0x0054(0x0004) (ZeroConstructor, IsPlainOldData)
	int                                                Item;                                                     // 0x0058(0x0004) (ZeroConstructor, IsPlainOldData)
	char pad_3424[0x18];
	struct FName                                       BoneName;                                                 // 0x0074(0x0008) (ZeroConstructor, IsPlainOldData)
	int                                                FaceIndex;                                                // 0x007C(0x0004) (ZeroConstructor, IsPlainOldData)
};

enum class EPlayerActivityType : uint8_t
{
	None = 0,
	Bailing = 1,
	Cannon = 2,
	Cannon_END = 3,
	Capstan = 4,
	Capstan_END = 5,
	CarryingBooty = 6,
	CarryingBooty_END = 7,
	Dead = 8,
	Dead_END = 9,
	Digging = 10,
	Dousing = 11,
	EmptyingBucket = 12,
	Harpoon = 13,
	Harpoon_END = 14,
	LoseHealth = 15,
	Repairing = 16,
	Sails = 17,
	Sails_END = 18,
	UndoingRepair = 19,
	Wheel = 20,
	Wheel_END = 21,
};

//struct APlayerState
//{
//	char pad[0x03c8];
//	float                                              Score;                                                     // 0x03D0(0x0004) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
//	float											   Ping;                                                      // 0x03D4(0x0001) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
//	//FString							                   PlayerName;                                                // 0x03D8 (BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, RepNotify, HasGetValueTypeHash)
//	struct FString PlayerName; // 0x3d0(0x10)
//
//	EPlayerActivityType GetPlayerActivity()
//	{
//		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaPlayerState.GetPlayerActivity");
//		EPlayerActivityType activity;
//		ProcessEvent(this, fn, &activity);
//		return activity;
//	}
//};
// Class Engine.SceneComponent
// Size: 0x2e0 (Inherited: 0xc8)
// ScriptStruct Engine.ActorComponentTickFunction
// Size: 0x50 (Inherited: 0x48)
// ScriptStruct Engine.TickFunction
// Size: 0x48 (Inherited: 0x00)
struct FTickFunction {
	char UnknownData_0[0x30]; // 0x00(0x30)
	char TickGroup; // 0x30(0x01)
	char UnknownData_31[0x1]; // 0x31(0x01)
	char bTickEvenWhenPaused : 1; // 0x32(0x01)
	char bCanEverTick : 1; // 0x32(0x01)
	char bStartWithTickEnabled : 1; // 0x32(0x01)
	char bAllowTickOnDedicatedServer : 1; // 0x32(0x01)
	char UnknownData_32_4 : 4; // 0x32(0x01)
	char UnknownData_33[0xd]; // 0x33(0x0d)
	float TickInterval; // 0x40(0x04)
	char UnknownData_44[0x4]; // 0x44(0x04)
};

struct FActorComponentTickFunction : FTickFunction {
	char UnknownData_48[0x8]; // 0x48(0x08)
};
struct UActorComponent : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
	struct FActorComponentTickFunction PrimaryComponentTick; // 0x30(0x50)
	struct TArray<struct FName> ComponentTags; // 0x80(0x10)
	struct TArray<struct FSimpleMemberReference> UCSModifiedProperties; // 0x90(0x10)
	char UnknownData_A0[0x10]; // 0xa0(0x10)
	struct TArray<struct UAssetUserData*> AssetUserData; // 0xb0(0x10)
	char UnknownData_C0_0 : 3; // 0xc0(0x01)
	char bReplicates : 1; // 0xc0(0x01)
	char bNetAddressable : 1; // 0xc0(0x01)
	char UnknownData_C0_5 : 3; // 0xc0(0x01)
	char UnknownData_C1_0 : 6; // 0xc1(0x01)
	char bCreatedByConstructionScript : 1; // 0xc1(0x01)
	char bInstanceComponent : 1; // 0xc1(0x01)
	char bAutoActivate : 1; // 0xc2(0x01)
	char bIsActive : 1; // 0xc2(0x01)
	char bEditableWhenInherited : 1; // 0xc2(0x01)
	char UnknownData_C2_3 : 5; // 0xc2(0x01)
	char UnknownData_C3_0 : 3; // 0xc3(0x01)
	char bNeedsLoadForClient : 1; // 0xc3(0x01)
	char bNeedsLoadForServer : 1; // 0xc3(0x01)
	char UnknownData_C3_5 : 3; // 0xc3(0x01)
	char TargetNetDormancy; // 0xc4(0x01)
	char CreationMethod; // 0xc5(0x01)
	char UnknownData_C6[0x2]; // 0xc6(0x02)
};
// Class Engine.SceneComponent
// Size: 0x2e0 (Inherited: 0xc8)
struct USceneComponent : UActorComponent {
	struct TArray<struct USceneComponent*> AttachChildren; // 0xc8(0x10)
	char bRequiresCustomLocation : 1; // 0xd8(0x01)
	char UnknownData_D8_1 : 7; // 0xd8(0x01)
	char UnknownData_D9[0x7]; // 0xd9(0x07)
	struct USceneComponent* AttachParent; // 0xe0(0x08)
	struct FName AttachSocketName; // 0xe8(0x08)
	char bAbsoluteLocation : 1; // 0xf0(0x01)
	char bAbsoluteRotation : 1; // 0xf0(0x01)
	char bAbsoluteScale : 1; // 0xf0(0x01)
	char bVisible : 1; // 0xf0(0x01)
	char bAbsoluteTranslation : 1; // 0xf0(0x01)
	char bHiddenInGame : 1; // 0xf0(0x01)
	char bShouldUpdatePhysicsVolume : 1; // 0xf0(0x01)
	char bBoundsChangeTriggersStreamingDataRebuild : 1; // 0xf0(0x01)
	char bUseAttachParentBound : 1; // 0xf1(0x01)
	char bComponentIsUnionOfChildren : 1; // 0xf1(0x01)
	char UnknownData_F1_2 : 1; // 0xf1(0x01)
	char bJitterReduction : 1; // 0xf1(0x01)
	char UnknownData_F1_4 : 1; // 0xf1(0x01)
	char CacheRootVelocity : 1; // 0xf1(0x01)
	char bWorldToComponentUpdated : 1; // 0xf1(0x01)
	char UnknownData_F2[0xe]; // 0xf2(0x0e)
	struct APhysicsVolume* PhysicsVolume; // 0x100(0x08)
	struct FVector RelativeLocation; // 0x108(0x0c)
	struct FRotator RelativeRotation; // 0x114(0x0c)
	struct FVector RelativeScale3D; // 0x120(0x0c)
	char UnknownData_12C[0x104]; // 0x12c(0x104)
	char Mobility; // 0x230(0x01)
	char DetailMode; // 0x231(0x01)
	char pad1[0xAE];
};
// Class Engine.PlayerState
// Size: 0x450 (Inherited: 0x3c8)
struct APlayerState {
	char pad_00[0x3c8];
	float Score; // 0x3c8(0x04)
	char Ping; // 0x3cc(0x01)
	char UnknownData_3CD[0x3]; // 0x3cd(0x03)
	struct FString PlayerName; // 0x3d0(0x10)
	char UnknownData_3E0[0x10]; // 0x3e0(0x10)
	int32_t PlayerId; // 0x3f0(0x04)
	char bIsSpectator : 1; // 0x3f4(0x01)
	char bOnlySpectator : 1; // 0x3f4(0x01)
	char bIsABot : 1; // 0x3f4(0x01)
	char UnknownData_3F4_3 : 1; // 0x3f4(0x01)
	char bIsInactive : 1; // 0x3f4(0x01)
	char bFromPreviousLevel : 1; // 0x3f4(0x01)
	char UnknownData_3F4_6 : 2; // 0x3f4(0x01)
	char UnknownData_3F5[0x3]; // 0x3f5(0x03)
	int32_t StartTime; // 0x3f8(0x04)
	char UnknownData_3FC[0x4]; // 0x3fc(0x04)
	struct UClass* EngineMessageClass; // 0x400(0x08)
	char UnknownData_408[0x18]; // 0x408(0x18)
	char UniqueId[0x18]; // 0x420(0x18)
	char UnknownData_438[0x18]; // 0x438(0x18)

	void OnRep_UniqueId(); // Function Engine.PlayerState.OnRep_UniqueId // Native|Public // @ game+0x2ceae70
};


struct ADamageZone {
	char pad[0x0654];
	int32_t DamageLevel; // 0x654(0x04)
};

struct AHullDamage {
	char pad[0x0410];
	struct TArray<struct ADamageZone*> DamageZones; // 0x410(0x10)
	TArray<class ACharacter*> ActiveHullDamageZones; // 0x0420
};

struct AShipInternalWater {
	float GetNormalizedWaterAmount() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipInternalWater.GetNormalizedWaterAmount");
		float params = 0.f;
		ProcessEvent(this, fn, &params);
		return params;
	}

	void AddWaterAmount(float WaterToAdd) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipInternalWater.AddWaterAmount");
		struct {
			float WaterToAdd;
		} params;
		params.WaterToAdd = WaterToAdd;
		ProcessEvent(this, fn, &params);
	}
};

struct AFauna {
	char pad1[0x0818];
	FText DisplayName; // 0x0818
};

// ScriptStruct Engine.RepMovement
// Size: 0x38 (Inherited: 0x00)
struct FRepMovement {
	struct FVector LinearVelocity; // 0x00(0x0c)
	struct FVector AngularVelocity; // 0x0c(0x0c)
	struct FVector Location; // 0x18(0x0c)
	struct FRotator Rotation; // 0x24(0x0c)
	char bSimulatedPhysicSleep : 1; // 0x30(0x01)
	char bRepPhysics : 1; // 0x30(0x01)
	char UnknownData_30_2 : 6; // 0x30(0x01)
	char LocationQuantizationLevel; // 0x31(0x01)
	char VelocityQuantizationLevel; // 0x32(0x01)
	char RotationQuantizationLevel; // 0x33(0x01)
	char UnknownData_34[0x4]; // 0x34(0x04)
};

struct AShipReplicatedM {
	char pad_0[0x90];
	struct FRepMovement ReplicatedMovement; // 0x94(0x38)
};
// Class Engine.Actor
// Size: 0x3d0 (Inherited: 0x28)


struct AActor : UObject {
	char PrimaryActorTick[0x50]; // 0x28(0x50)
	float CustomTimeDilation; // 0x78(0x04)
	char bAllowRemovalFromServerWhenCollisionMerged : 1; // 0x7c(0x01)
	char bAllowRemovalFromServerWhenAutomaticallyInstanced : 1; // 0x7c(0x01)
	char bHidden : 1; // 0x7c(0x01)
	char bNetTemporary : 1; // 0x7c(0x01)
	char bNetStartup : 1; // 0x7c(0x01)
	char bOnlyRelevantToOwner : 1; // 0x7c(0x01)
	char bAlwaysRelevant : 1; // 0x7c(0x01)
	char bReplicateMovement : 1; // 0x7c(0x01)
	char bTearOff : 1; // 0x7d(0x01)
	char bExchangedRoles : 1; // 0x7d(0x01)
	char bPendingNetUpdate : 1; // 0x7d(0x01)
	char bNetLoadOnClient : 1; // 0x7d(0x01)
	char bNetUseOwnerRelevancy : 1; // 0x7d(0x01)
	char bBlockInput : 1; // 0x7d(0x01)
	char UnknownData_7D_6 : 1; // 0x7d(0x01)
	char bCanBeInCluster : 1; // 0x7d(0x01)
	char UnknownData_7E_0 : 2; // 0x7e(0x01)
	char bActorEnableCollision : 1; // 0x7e(0x01)
	char UnknownData_7E_3 : 1; // 0x7e(0x01)
	char bReplicateAttachment : 1; // 0x7e(0x01)
	char UnknownData_7E_5 : 1; // 0x7e(0x01)
	char bReplicates : 1; // 0x7e(0x01)
	char UnknownData_7F[0x1]; // 0x7f(0x01)
	char OnPreNetOwnershipChange[0x1]; // 0x80(0x01)
	char UnknownData_81[0x1]; // 0x81(0x01)
	char RemoteRole; // 0x82(0x01)
	char UnknownData_83[0x5]; // 0x83(0x05)
	struct AActor* Owner; // 0x88(0x08)
	char SpawnRestrictions; // 0x90(0x01)
	char UnknownData_91[0x3]; // 0x91(0x03)
	struct FRepMovement ReplicatedMovement; // 0x94(0x38)
	char UnknownData_CC[0x4]; // 0xcc(0x04)
	char AttachmentReplication[0x48]; // 0xd0(0x48)
	char Role; // 0x118(0x01)
	char UnknownData_119[0x1]; // 0x119(0x01)
	char AutoReceiveInput; // 0x11a(0x01)
	char UnknownData_11B[0x1]; // 0x11b(0x01)
	int32_t InputPriority; // 0x11c(0x04)
	struct UInputComponent* InputComponent; // 0x120(0x08)
	float NetCullDistanceSquared; // 0x128(0x04)
	char UnknownData_12C[0x4]; // 0x12c(0x04)
	int32_t NetTag; // 0x130(0x04)
	float NetUpdateTime; // 0x134(0x04)
	float NetUpdateFrequency; // 0x138(0x04)
	float NetPriority; // 0x13c(0x04)
	float LastNetUpdateTime; // 0x140(0x04)
	struct FName NetDriverName; // 0x144(0x08)
	char bAutoDestroyWhenFinished : 1; // 0x14c(0x01)
	char bCanBeDamaged : 1; // 0x14c(0x01)
	char bActorIsBeingDestroyed : 1; // 0x14c(0x01)
	char bCollideWhenPlacing : 1; // 0x14c(0x01)
	char bFindCameraComponentWhenViewTarget : 1; // 0x14c(0x01)
	char bRelevantForNetworkReplays : 1; // 0x14c(0x01)
	char UnknownData_14C_6 : 2; // 0x14c(0x01)
	char UnknownData_14D[0x3]; // 0x14d(0x03)
	char SpawnCollisionHandlingMethod; // 0x150(0x01)
	char UnknownData_151[0x7]; // 0x151(0x07)
	struct APawn* Instigator; // 0x158(0x08)
	struct TArray<struct AActor*> Children; // 0x160(0x10)
	struct USceneComponent* RootComponent; // 0x170(0x08)
	struct TArray<struct AMatineeActor*> ControllingMatineeActors; // 0x178(0x10)
	float InitialLifeSpan; // 0x188(0x04)
	char UnknownData_18C[0x4]; // 0x18c(0x04)
	char bAllowReceiveTickEventOnDedicatedServer : 1; // 0x190(0x01)
	char UnknownData_190_1 : 7; // 0x190(0x01)
	char UnknownData_191[0x7]; // 0x191(0x07)
	struct TArray<struct FName> Layers; // 0x198(0x10)
	char ParentComponentActor[0x8]; // 0x1a8(0x08)
	struct TArray<struct AActor*> ChildComponentActors; // 0x1b0(0x10)
	char UnknownData_1C0[0x8]; // 0x1c0(0x08)
	char bActorSeamlessTraveled : 1; // 0x1c8(0x01)
	char bIgnoresOriginShifting : 1; // 0x1c8(0x01)
	char bEnableAutoLODGeneration : 1; // 0x1c8(0x01)
	char InvertFeatureCheck : 1; // 0x1c8(0x01)
	char UnknownData_1C8_4 : 4; // 0x1c8(0x01)
	char UnknownData_1C9[0x3]; // 0x1c9(0x03)
	struct FName Feature; // 0x1cc(0x08)
	char UnknownData_1D4[0x4]; // 0x1d4(0x04)
	struct TArray<struct FName> Tags; // 0x1d8(0x10)
	uint64_t HiddenEditorViews; // 0x1e8(0x08)
	char UnknownData_1F0[0x4]; // 0x1f0(0x04)
	char UnknownData_1F4[0x3c]; // 0x1f4(0x3c)
	char OnEndPlay[0x1]; // 0x230(0x01)
	bool bDoOverlapNotifiesOnLoad; // 0x231(0x01)
	char UnknownData_232[0xf6]; // 0x232(0xf6)
	struct TArray<struct UActorComponent*> BlueprintCreatedComponents; // 0x328(0x10)
	struct TArray<struct UActorComponent*> InstanceComponents; // 0x338(0x10)
	char UnknownData_348[0x8]; // 0x348(0x08)
	struct TArray<struct AActor*> ChildActorInterfaceProviders; // 0x350(0x10)
	char UnknownData_360[0x68]; // 0x360(0x68)
	double DormancyLingeringInSeconds; // 0x3c8(0x08)

	struct FVector GetActorRightVector()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorRightVector");
		FVector ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}
	void SetActorScale3D(struct FVector NewScale3D)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.SetActorScale3D");
		struct
		{
			FVector NewScale3D;
		}params;
		params.NewScale3D = NewScale3D;
		ProcessEvent(this, fn, &params);
	} // Function Engine.Actor.SetActorScale3D // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2bb1610
};

struct ANamedPawn {
	char pad[0x3e8];
	struct APlayerState* PlayerState; // 0x3e8(0x08)
};
// Class Engine.Pawn
// Size: 0x448 (Inherited: 0x3d0)
struct APawn : AActor {
	char pad[0x20];
	struct APlayerState* PlayerState; // 0x3f0(0x08)
	char pad2[0x50];
};

struct FWorldMapIslandDataCaptureParams
{
	char pad1[0x0018];
	struct FVector                                     WorldSpaceCameraPosition;                                  // 0x0018(0x000C) (Edit, ZeroConstructor, Transient, EditConst, IsPlainOldData, NoDestructor)
	char pad2[0x8];
	float                                              CameraOrthoWidth;                                          // 0x002C(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)

};

struct UWorldMapIslandDataAsset {
	char pad[0x0030];
	struct FWorldMapIslandDataCaptureParams            CaptureParams;                                             // 0x0030(0x0040) (Edit, BlueprintVisible, BlueprintReadOnly)
	FVector WorldSpaceCameraPosition;
	// ADD THE OFFSET OF CAPTUREPARAMS TO THIS OFFSET
};

// Class Athena.IslandDataAssetEntry
// Size: 0x118 (Inherited: 0x28)
struct UIslandDataAssetEntry {
	char pad_00[0x28];
	struct FName IslandName; // 0x28(0x08)
	struct TArray<struct FTreasureMapData> TreasureMaps; // 0x30(0x10)
	struct UWorldMapIslandDataAsset* WorldMapData; // 0x40(0x08)
	struct UGeneratedLocationsDataAsset* UndergroundTreasureLocationsSource; // 0x48(0x08)
	struct TArray<struct FVector> UndergroundTreasureLocations; // 0x50(0x10)
	struct ULandmarkTreasureLocationsDataAsset* LandmarkTreasureLocationsSource; // 0x60(0x08)
	struct UGeneratedLocationsDataAsset* AISpawnLocationsSource; // 0x68(0x08)
	struct TArray<struct FVector> AISpawnLocations; // 0x70(0x10)
	struct TArray<struct UIslandItemDataAsset*> IslandItemLocationDataSources; // 0x80(0x10)
	struct TArray<struct UIslandSalvageSpawnerCollection*> IslandSalvageSpawnerCollections; // 0x90(0x10)
	struct TArray<struct FTypedIslandItemSpawnLocationData> SalvageItemsLocationData; // 0xa0(0x10)
	FString* LocalisedName; // 0xb0(0x38)
	struct UAISpawner* AISpawner; // 0xe8(0x08)
	struct UAISpawner* CannonsAISpawner; // 0xf0(0x08)
	struct UAISpawner* EmergentCaptainSpawner; // 0xf8(0x08)
	struct UIslandMaterialStatusZone* IslandMaterialStatusZone; // 0x100(0x08)
	char StatToFireWhenPlayerSetsFootOnIsland[0x4]; // 0x108(0x04)
	char StatToFireWhenShipVisitsAnIsland[0x4]; // 0x10c(0x04)
	bool ShouldBeHiddenOnWorldMap; // 0x110(0x01)
	bool UseAdvancedSearchForMermaidSpawn; // 0x111(0x01)
	char IslandActiveEventType; // 0x112(0x01)
	char UnknownData_113[0x5]; // 0x113(0x05)
};

struct UIslandDataAsset {
	char pad[0x0048];
	struct TArray<struct UIslandDataAssetEntry*> IslandDataEntries; // 0x48(0x10)
};

struct AIslandService {
	char pad[0x0458];
	UIslandDataAsset* IslandDataAsset; // 0x458
};

struct FGuid
{
	int                                                A;                                                         // 0x0000(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                B;                                                         // 0x0004(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                C;                                                         // 0x0008(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                D;                                                         // 0x000C(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

// ScriptStruct Athena.WorldMapShipLocation
// Size: 0x40 (Inherited: 0x00)
struct FWorldMapShipLocation {
	struct FGuid CrewId; // 0x00(0x10)
	struct UClass* ShipSize; // 0x10(0x08)
	struct FVector2D Location; // 0x18(0x08)
	float Rotation; // 0x20(0x04)
	char ReplicatedRotation; // 0x24(0x01)
	char Flags; // 0x25(0x01)
	char UnknownData_26[0x2]; // 0x26(0x02)
	struct UTexture* CrewLiveryOverlayIcon; // 0x28(0x08)
	char ReapersMarkLevel; // 0x30(0x01)
	char EmissaryLevel; // 0x31(0x01)
	bool OwnerIsInFaction; // 0x32(0x01)
	bool OwnerIsMaxFaction; // 0x33(0x01)
	struct FName OwnerFactionName; // 0x34(0x08)
	bool OwnerIsInvadingShip; // 0x3c(0x01)
	char UnknownData_3D[0x3]; // 0x3d(0x03)
};

struct FSessionTemplate
{
	struct FString                                     TemplateName;                                              // 0x0000(0x0010) (ZeroConstructor, Protected, HasGetValueTypeHash)
	unsigned char             SessionType;                                               // 0x0010(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	unsigned char                                      UnknownData_2Q1C[0x3];                                     // 0x0011(0x0003) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	int                                                MaxPlayers;                                                // 0x0014(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)

};

// ScriptStruct Sessions.CrewSessionTemplate
// 0x0020 (0x0038 - 0x0018)
struct FCrewSessionTemplate : public FSessionTemplate
{
	struct FString                                     MatchmakingHopper;                                         // 0x0018(0x0010) (ZeroConstructor, HasGetValueTypeHash)
	class UClass* ShipSize;                                                  // 0x0028(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash)
	int                                                MaxMatchmakingPlayers;                                     // 0x0030(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	unsigned char                                      UnknownData_JPXK[0x4];                                     // 0x0034(0x0004) MISSED OFFSET (PADDING)

};

// ScriptStruct Athena.Crew
// Size: 0x98 (Inherited: 0x00)
//struct FCrew
//{
//	struct FGuid                                       CrewId;                                                   // 0x0000(0x0010) (ZeroConstructor, IsPlainOldData)
//	struct FGuid                                       SessionId;                                                // 0x0010(0x0010) (ZeroConstructor, IsPlainOldData)
//	TArray<class APlayerState*>                        Players;                                                  // 0x0020(0x0010) (ZeroConstructor)
//	struct FCrewSessionTemplate                        CrewSessionTemplate;                                      // 0x0030(0x0038)
//	struct FGuid                                       LiveryID;                                                 // 0x0068(0x0010) (ZeroConstructor, IsPlainOldData)
//	char pad[0x18];
//};

struct FCrew {
	struct FGuid CrewId; // 0x00(0x10)
	struct FGuid SessionId; // 0x10(0x10)
	struct TArray<struct APlayerState*> Players; // 0x20(0x10)
	struct FCrewSessionTemplate CrewSessionTemplate; // 0x30(0x38)
	struct FGuid LiveryID; // 0x68(0x10)
	char UnknownData_78[0x8]; // 0x78(0x08)
	struct TArray<struct AActor*> AssociatedActors; // 0x80(0x10)
	bool HasEverSetSail; // 0x90(0x01)
	char UnknownData_91[0x3]; // 0x91(0x03)
	int32_t ScrambleNameIndex; // 0x94(0x04)
};

struct ACrewService {
	char pad[0x04A0];
	struct TArray<struct FCrew> Crews; // 0x4a0(0x10)
};

struct AShipService
{
	int GetNumShips()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipService.GetNumShips");
		int num;
		ProcessEvent(this, fn, &num);
		return num;
	}
};

// Class Athena.AthenaGameState
// Size: 0xb80 (Inherited: 0x540)
struct AAthenaGameState {
	char UnknownData_00[0x540]; // 0x0(0x540)
	char UnknownData_540[0x78]; // 0x540(0x78)
	struct AWindService* WindService; // 0x5b8(0x08)
	struct APlayerManagerService* PlayerManagerService; // 0x5c0(0x08)
	struct AShipService* ShipService; // 0x5c8(0x08)
	struct AWatercraftService* WatercraftService; // 0x5d0(0x08)
	struct ATimeService* TimeService; // 0x5d8(0x08)
	struct UHealthCustomizationService* HealthService; // 0x5e0(0x08)
	struct UCustomWeatherService* CustomWeatherService; // 0x5e8(0x08)
	struct UCustomStatusesService* CustomStatusesService; // 0x5f0(0x08)
	struct AFFTWaterService* WaterService; // 0x5f8(0x08)
	struct AStormService* StormService; // 0x600(0x08)
	struct ACrewService* CrewService; // 0x608(0x08)
	struct ACaptainedSessionService* CaptainedSessionService; // 0x610(0x08)
	struct AContestZoneService* ContestZoneService; // 0x618(0x08)
	struct AContestRowboatsService* ContestRowboatsService; // 0x620(0x08)
	struct AIslandService* IslandService; // 0x628(0x08)
	struct ANPCService* NPCService; // 0x630(0x08)
	struct ASkellyFortService* SkellyFortService; // 0x638(0x08)
	struct ADeepSeaRegionService* DeepSeaRegionService; // 0x640(0x08)
	struct AAIDioramaService* AIDioramaService; // 0x648(0x08)
	struct AAshenLordEncounterService* AshenLordEncounterService; // 0x650(0x08)
	struct AAggressiveGhostShipsEncounterService* AggressiveGhostShipsEncounterService; // 0x658(0x08)
	struct ATallTaleService* TallTaleService; // 0x660(0x08)
	struct AAIShipObstacleService* AIShipObstacleService; // 0x668(0x08)
	struct AAIShipService* AIShipService; // 0x670(0x08)
	struct AAITargetService* AITargetService; // 0x678(0x08)
	struct UShipLiveryCatalogueService* ShipLiveryCatalogueService; // 0x680(0x08)
	struct ADrawDebugService* DrawDebugService; // 0x688(0x08)
	struct AWorldEventZoneService* WorldEventZoneService; // 0x690(0x08)
	struct UWorldResourceRegistry* WorldResourceRegistry; // 0x698(0x08)
	struct AKrakenService* KrakenService; // 0x6a0(0x08)
	struct UPlayerNameService* PlayerNameService; // 0x6a8(0x08)
	struct ATinySharkService* TinySharkService; // 0x6b0(0x08)
	struct AProjectileService* ProjectileService; // 0x6b8(0x08)
	struct ULaunchableProjectileService* LaunchableProjectileService; // 0x6c0(0x08)
	struct UServerNotificationsService* ServerNotificationsService; // 0x6c8(0x08)
	struct AAIManagerService* AIManagerService; // 0x6d0(0x08)
	struct AAIEncounterService* AIEncounterService; // 0x6d8(0x08)
	struct AAIEncounterGenerationService* AIEncounterGenerationService; // 0x6e0(0x08)
	struct UEncounterService* EncounterService; // 0x6e8(0x08)
	struct UGameEventSchedulerService* GameEventSchedulerService; // 0x6f0(0x08)
	struct UHideoutService* HideoutService; // 0x6f8(0x08)
	struct UAthenaStreamedLevelService* StreamedLevelService; // 0x700(0x08)
	struct ULocationProviderService* LocationProviderService; // 0x708(0x08)
	struct AHoleService* HoleService; // 0x710(0x08)
	struct APlayerBuriedItemService* PlayerBuriedItemService; // 0x718(0x08)
	struct ULoadoutService* LoadoutService; // 0x720(0x08)
	struct UOcclusionService* OcclusionService; // 0x728(0x08)
	struct UPetsService* PetsService; // 0x730(0x08)
	struct UAthenaAITeamsService* AthenaAITeamsService; // 0x738(0x08)
	struct AAllianceService* AllianceService; // 0x740(0x08)
	struct UMaterialAccessibilityService* MaterialAccessibilityService; // 0x748(0x08)
	struct UNPCLoadedOnClientService* NPCLoadedOnClientService; // 0x750(0x08)
	struct AReapersMarkService* ReapersMarkService; // 0x758(0x08)
	struct AEmissaryLevelService* EmissaryLevelService; // 0x760(0x08)
	struct AFactionService* FactionService; // 0x768(0x08)
	struct ACampaignService* CampaignService; // 0x770(0x08)
	struct AStoryService* StoryService; // 0x778(0x08)
	struct AStorySpawnedActorsService* StorySpawnedActorsService; // 0x780(0x08)
	struct AStoryClaimedResourcesService* StoryClaimedResourcesService; // 0x788(0x08)
	struct UGlobalVoyageDirectorService* GlobalVoyageDirector; // 0x790(0x08)
	struct AFlamesOfFateSettingsService* FlamesOfFateSettingsService; // 0x798(0x08)
	struct AServiceStatusNotificationsService* ServiceStatusNotificationsService; // 0x7a0(0x08)
	struct UMigrationService* MigrationService; // 0x7a8(0x08)
	struct UShipStockService* ShipStockService; // 0x7b0(0x08)
	struct AShroudBreakerService* ShroudBreakerService; // 0x7b8(0x08)
	struct UServerUpdateReportingService* ServerUpdateReportingService; // 0x7c0(0x08)
	struct AGenericMarkerService* GenericMarkerService; // 0x7c8(0x08)
	struct AMechanismsService* MechanismsService; // 0x7d0(0x08)
	struct UMerchantContractsService* MerchantContractsService; // 0x7d8(0x08)
	struct UShipFactory* ShipFactory; // 0x7e0(0x08)
	struct URewindPhysicsService* RewindPhysicsService; // 0x7e8(0x08)
	struct UNotificationMessagesDataAsset* NotificationMessagesDataAsset; // 0x7f0(0x08)
	struct AProjectileCooldownService* ProjectileCooldownService; // 0x7f8(0x08)
	struct UIslandReservationService* IslandReservationService; // 0x800(0x08)
	struct APortalService* PortalService; // 0x808(0x08)
	struct UMeshMemoryConstraintService* MeshMemoryConstraintService; // 0x810(0x08)
	struct ABootyStorageService* BootyStorageService; // 0x818(0x08)
	struct ALoadoutCostService* LoadoutCostService; // 0x820(0x08)
	struct ASpireService* SpireService; // 0x828(0x08)
	struct AFireworkService* FireworkService; // 0x830(0x08)
	struct AInvasionService* InvasionService; // 0x838(0x08)
	struct UAirGivingService* AirGivingService; // 0x840(0x08)
	struct UCutsceneService* CutsceneService; // 0x848(0x08)
	struct ACargoRunService* CargoRunService; // 0x850(0x08)
	struct ACommodityDemandService* CommodityDemandService; // 0x858(0x08)
	struct ADebugTeleportationDestinationService* DebugTeleportationDestinationService; // 0x860(0x08)
	struct ASeasonProgressionUIService* SeasonProgressionUIService; // 0x868(0x08)
	struct UTransientActorService* TransientActorService; // 0x870(0x08)
	struct ATunnelsOfTheDamnedService* TunnelsOfTheDamnedService; // 0x878(0x08)
	struct UWorldSequenceService* WorldSequenceService; // 0x880(0x08)
	struct UItemLifetimeManagerService* ItemLifetimeManagerService; // 0x888(0x08)
	struct AShipStorageJettisonService* ShipStorageJettisonService; // 0x890(0x08)
	struct ASeaFortsService* SeaFortsService; // 0x898(0x08)
	struct ACustomServerLocalisationService* CustomServerLocalisationService; // 0x8a0(0x08)
	struct ABeckonService* BeckonService; // 0x8a8(0x08)
	struct UVolcanoService* VolcanoService; // 0x8b0(0x08)
	struct UShipAnnouncementService* ShipAnnouncementService; // 0x8b8(0x08)
	struct AShipsLogService* ShipsLogService; // 0x8c0(0x08)
	struct UAsyncLoadingMonitoringService* AsyncLoadingMonitoringService; // 0x8c8(0x08)
	struct AActorOfInterestService* ActorOfInterestService; // 0x8d0(0x08)
	struct AUserSettingsService* UserSettingsService; // 0x8d8(0x08)
	char UnknownData_8E0[0x180]; // 0x8e0(0x180)
	char ServiceCoordinator[0x20]; // 0xa60(0x20)
	char UnknownData_A80[0x28]; // 0xa80(0x28)
	char ChatComponents[0x10]; // 0xaa8(0x10)
	char UnknownData_AB8[0x80]; // 0xab8(0x80)
	bool IsXboxGamePadOnlyServer; // 0xb38(0x01)
	bool ShouldDisableAsyncOcclusionCheck; // 0xb39(0x01)
	char UnknownData_B3A[0x6]; // 0xb3a(0x06)
	struct FString SubPlayMode; // 0xb40(0x10)
	struct UCustomVaultService* CustomVaultService; // 0xb50(0x08)
	struct UEntityEnumerationService* EntityEnumerationService; // 0xb58(0x08)
	struct ULevelAssetCachingService* LevelAssetCachingService; // 0xb60(0x08)
	struct UCrewSkillRatingService* CrewSkillRatingService; // 0xb68(0x08)
	char UnknownData_B70[0x18]; // 0xb70(0x18)
};

struct UItemDesc {
	char pad_28[0x28];
	FString* Title; // 0x28(0x38)
	char pad_c0[0xc0];
};

struct UItemDescEx : UObject {
	struct FText Title; // 0x28(0x38)
	struct FText Description; // 0x60(0x38)
	//char pad_60[0x68];
	char pad_98[0x30];
	char pad_c0[0x58];
};

struct AItemInfo {
	char pad[0x440];
	UItemDesc* Desc; // 0x440(0x08)
};
// ScriptStruct Engine.Vector_NetQuantize10
// Size: 0x0c (Inherited: 0x0c)
struct FVector_NetQuantize10 : FVector {
};
// ScriptStruct Engine.RepRootMotionMontage
// Size: 0x40 (Inherited: 0x00)
struct FRepRootMotionMontage {
	struct UAnimMontage* AnimMontage; // 0x00(0x08)
	float Position; // 0x08(0x04)
	struct FVector_NetQuantize10 Location; // 0x0c(0x0c)
	struct FRotator Rotation; // 0x18(0x0c)
	char UnknownData_24[0x4]; // 0x24(0x04)
	struct UPrimitiveComponent* MovementBase; // 0x28(0x08)
	struct FName MovementBaseBoneName; // 0x30(0x08)
	bool bRelativePosition; // 0x38(0x01)
	bool bRelativeRotation; // 0x39(0x01)
	char UnknownData_3A[0x6]; // 0x3a(0x06)
};
// ScriptStruct Engine.Vector_NetQuantize100
// Size: 0x0c (Inherited: 0x0c)
struct FVector_NetQuantize100 : FVector {
};

// ScriptStruct Engine.BasedMovementInfo
// Size: 0x38 (Inherited: 0x00)
struct FBasedMovementInfo {
	struct UPrimitiveComponent* MovementBase; // 0x00(0x08)
	struct FName BoneName; // 0x08(0x08)
	struct FVector_NetQuantize100 Location; // 0x10(0x0c)
	struct FRotator Rotation; // 0x1c(0x0c)
	bool bServerHasBaseComponent; // 0x28(0x01)
	bool bRelativeRotation; // 0x29(0x01)
	char UnknownData_2A[0x2]; // 0x2a(0x02)
	struct FVector LinearVelocity; // 0x2c(0x0c)
};
// ScriptStruct Engine.RootMotionMovementParams
// Size: 0x40 (Inherited: 0x00)
struct FRootMotionMovementParams {
	bool bHasRootMotion; // 0x00(0x01)
	char UnknownData_1[0x3]; // 0x01(0x03)
	float BlendWeight; // 0x04(0x04)
	char UnknownData_8[0x8]; // 0x08(0x08)
	struct FTransform RootMotionTransform; // 0x10(0x30)
};

// Class Engine.Character
// Size: 0x5e0 (Inherited: 0x440)
struct ACharacter : APawn {
	//struct USkeletalMeshComponent* Mesh; // 0x440(0x08)
	char pad[0x198];
	
	bool IsLoading() {
		static auto fn = UObject::FindObject<UFunction>("Function AthenaLoadingScreen.AthenaLoadingScreenBlueprintFunctionLibrary.IsLoadingScreenVisible");
		bool isLoading = true;
		ProcessEvent(this, fn, &isLoading);
		return isLoading;
	}

	inline bool isItem() {
		static auto obj = UObject::FindClass("Class Athena.ItemProxy");
		return IsA(obj);
	}
	inline bool isLight() {
		static auto obj = UObject::FindClass("Class Engine.SkyLight");
		return IsA(obj);
	}

	inline bool isFarShip() {
		static auto obj = UObject::FindClass("Class Athena.ShipNetProxy");
		return IsA(obj);
	}
	inline bool isDrunknessComponent() {
		static auto obj = UObject::FindClass("Class Athena.DrunkennessComponent");
		return IsA(obj);
	}

	inline bool isShip() {
		static auto obj = UObject::FindClass("Class Athena.Ship");
		return IsA(obj);
	}
	inline bool isLightingCompoent() {
		static auto obj = UObject::FindClass("Class Engine.LightComponentBase");
		return IsA(obj);
	}
	
	inline bool isGhostShip() {
		static auto obj = UObject::FindClass("Class Athena.AggressiveGhostShip");
		return IsA(obj);
	}
	inline bool isAthenaCharacter() {
		static auto obj = UObject::FindClass("Class Athena.AthenaCharacter");
		return IsA(obj);
	}

	inline bool isSkeleton() {
		static auto obj = FindClass("Class Athena.AthenaAICharacter");
		return IsA(obj);
	}
	bool CanJump() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Character.CanJump");
		bool Canjump = false;
		ProcessEvent(this, fn, &Canjump);
		return Canjump;
	}
	void Jump() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Character.Jump");
		bool jump = false;
		ProcessEvent(this, fn, &jump);
	}
	inline bool isPlayer() {
		static auto obj = UObject::FindClass("Class Athena.AthenaPlayerCharacter");
		return IsA(obj);
	}

	inline bool isAmmoChest() {
		static auto obj = FindClass("Class Athena.AmmoChest");
		return IsA(obj);
	}

	inline bool isXMarkMap() {
		static auto obj = UObject::FindClass("Class Athena.XMarksTheSpotMap");
		return IsA(obj);
	}
	inline bool isBp_playerpirate() {
		static auto obj = UObject::FindClass("BlueprintGeneratedClass BP_PlayerPirate.BP_PlayerPirate_C");
		return IsA(obj);
	}
	
 
	inline bool isShipwreck() {
		static auto obj = UObject::FindClass("Class Athena.Shipwreck");
		return IsA(obj);
	}

	inline bool isMermaid() {
		static auto obj = UObject::FindClass("Class Athena.Mermaid");
		return IsA(obj);
	}

	inline bool isRowboat() {
		static auto obj = UObject::FindClass("Class Watercrafts.Rowboat");
		return IsA(obj);
	}

	inline bool isShark() {
		static auto obj = UObject::FindClass("Class Athena.SharkPawn");
		return IsA(obj);
	}

	inline bool isEvent() {
		static auto obj = UObject::FindClass("Class Athena.GameplayEventSignal");
		return IsA(obj);
	}

	inline bool isPuzzleVault() {
		static auto obj = UObject::FindClass("Class Athena.PuzzleVault");
		return IsA(obj);
	}

	inline bool isBarrel() {
		static auto obj = UObject::FindClass("Class Athena.StorageContainer");
		return IsA(obj);
	}

	inline bool isStorageComponent() {
		static auto obj = UObject::FindClass("Class Athena.StorageContainerComponent");
		return IsA(obj);
	}

	inline bool isAnimal() {
		static auto obj = UObject::FindClass("Class AthenaAI.Fauna");
		return IsA(obj);
	}

	bool IsDead() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.IsDead");
		bool isDead = true;
		ProcessEvent(this, fn, &isDead);
		return isDead;
	}

	bool IsInWater() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.IsInWater");
		bool isInWater = false;
		ProcessEvent(this, fn, &isInWater);
		return isInWater;
	}

	bool isWeapon() {
		static auto obj = UObject::FindClass("Class Athena.ProjectileWeapon");
		return IsA(obj);
	}

	bool isSpyglass() {
		static auto obj = UObject::FindClass("Class Athena.Spyglass");
		return IsA(obj);
	}

	inline bool isCannon() {
		static auto obj = UObject::FindClass("Class Athena.Cannon");
		return IsA(obj);
	}

	inline bool isMapTable() {
		static auto obj = UObject::FindClass("Class Athena.MapTable");
		return IsA(obj);
	}

	inline bool isCookingPot() {
		static auto obj = UObject::FindClass("Class Cooking.CookingPot");
		return IsA(obj);
	}

	inline bool isCookableComponent() {
		static auto obj = UObject::FindClass("Class Cooking.CookableComponent");
		return IsA(obj);
	}

	inline bool isLightningController() {
		static auto obj = UObject::FindClass("Class Athena.LightingController");
		return IsA(obj);
	}
	inline bool isClouDome() {
		static auto obj = UObject::FindClass("Class Athena.CloudDome");
		return IsA(obj);
	}
	inline bool isStorm() {
		static auto obj = UObject::FindClass("Class Athena.Storm");
		return IsA(obj);
	}
	inline bool isStormService() {
		static auto obj = UObject::FindClass("Class Athena.StormService");
		return IsA(obj);
	}
	inline bool isFog() {
		static auto obj = UObject::FindClass("Class Athena.FogBank");
		return IsA(obj);
	}
	
	inline bool isMast() {
		static auto obj = UObject::FindClass("Class Athena.Mast");
		return IsA(obj);
	}
	inline bool isSky() {
		static auto obj = UObject::FindClass("Class Engine.SkyLight");
		return IsA(obj);
	}
	inline bool isCannonProjectile() {
		static auto obj = UObject::FindClass("Class Athena.CannonProjectile");
		return IsA(obj);
	}

	AHullDamage* GetHullDamage() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Ship.GetHullDamage");
		AHullDamage* params = nullptr;
		ProcessEvent(this, fn, &params);
		return params;
	}

	AShipInternalWater* GetInternalWater() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Ship.GetInternalWater");
		AShipInternalWater* params = nullptr;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FVector GetActorForwardVector() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorForwardVector");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FVector GetActorUpVector() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorUpVector");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	ACharacter* GetParentActor() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetParentActor");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	};

	ACharacter* GetAttachParentActor() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetAttachParentActor");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}

	ACharacter* GetCurrentShip() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.GetCurrentShip");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}
	void ChangeDrunkenness(char Type, float DrunkennessChange) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.DrunkennessComponent.ChangeDrunkenness");
		struct {
			char T = 0;
			float Dchange = 0.f;
		}params;
		Type = params.T;
		DrunkennessChange = params.Dchange;
		ProcessEvent(this, fn, &params);
	};
	FVector GetVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetVelocity");
		FVector velocity;
		ProcessEvent(this, fn, &velocity);
		return velocity;
	}

	FVector GetForwardVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorForwardVector");
		FVector ForwardVelocity;
		ProcessEvent(this, fn, &ForwardVelocity);
		return ForwardVelocity;
	}

	FVector K2_GetActorLocation() {
		static auto fn = FindObject<UFunction>("Function Engine.Actor.K2_GetActorLocation");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FRotator K2_GetActorRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.K2_GetActorRotation");
		FRotator params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	AItemInfo* GetItemInfo() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ItemProxy.GetItemInfo");
		AItemInfo* info = nullptr;
		ProcessEvent(this, fn, &info);
		return info;
	}

	void GetActorBounds(bool bOnlyCollidingComponents, FVector& Origin, FVector& BoxExtent) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorBounds");
		struct
		{
			bool bOnlyCollidingComponents = false;
			FVector Origin;
			FVector BoxExtent;
		} params;

		params.bOnlyCollidingComponents = bOnlyCollidingComponents;

		ProcessEvent(this, fn, &params);

		Origin = params.Origin;
		BoxExtent = params.BoxExtent;
	}

	float GetTargetFOV(class AAthenaPlayerCharacter* Character) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.FOVHandlerFunctions.GetTargetFOV");
		struct
		{
			AAthenaPlayerCharacter* Character;
			float ReturnValue = 0.f;
		}params;
		params.Character = Character;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	inline bool compareName(const char* name)
	{
		auto type = GetName();
		if (type.find(name) != std::string::npos)
			return true;
		return false;
	}

	void SetTargetFOV(class AAthenaPlayerCharacter* Character, float TargetFOV) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.FOVHandlerFunctions.SetTargetFOV");
		struct
		{
			AAthenaPlayerCharacter* Character;
			float ReturnValue = 0.f;
		}params;
		params.Character = Character;
		params.ReturnValue = TargetFOV;
		ProcessEvent(this, fn, &params);
	}
};

// ScriptStruct Athena.StorageContainerNode
// Size: 0x10 (Inherited: 0x00)
struct FStorageContainerNode {
	struct UClass* ItemDesc; // 0x00(0x08)
	int32_t NumItems; // 0x08(0x04)
	char UnknownData_C[0x4]; // 0x0c(0x04)
};
// ScriptStruct Athena.StorageContainerBackingStore
// Size: 0x40 (Inherited: 0x00)
struct FStorageContainerBackingStore {
	char pad_0[0x20];
	struct TArray<struct FStorageContainerNode> ContainerNodes; // 0x20(0x10)
	bool AllowedItemsAreCached; // 0x30(0x01)
	char UnknownData_31[0x7]; // 0x31(0x07)
	char CachedAllowedItems[0x8]; // 0x38(0x08)
};

// Class Athena.StorageContainerComponent
// Size: 0x310 (Inherited: 0xc8)
struct UStorageContainerComponent {
	char pad_00[0xc8];
	char UnknownData_C8[0x18]; // 0xc8(0x18)
	char ContainerDisplayName[0x38]; // 0xe0(0x38)
	char UnknownData_118[0x8]; // 0x118(0x08)
	char InstanceTransform[0x30]; // 0x120(0x30)
	struct FStorageContainerBackingStore ContainerNodes; // 0x150(0x40)
	struct TArray<struct AActor*> QuickGivers; // 0x190(0x10)
	struct TArray<struct AActor*> QuickTakers; // 0x1a0(0x10)
	char AddItemSFX[0x8]; // 0x1b0(0x08)
	char TakeItemSFX[0x8]; // 0x1b8(0x08)
	char OpenContainerSFX[0x8]; // 0x1c0(0x08)
	char BeginQuickGiveSFX[0x8]; // 0x1c8(0x08)
	char EndQuickGiveSFX[0x8]; // 0x1d0(0x08)
	char BeginQuickTakeSFX[0x8]; // 0x1d8(0x08)
	char EndQuickTakeSFX[0x8]; // 0x1e0(0x08)
	char StorageContainerSelector[0x8]; // 0x1e8(0x08)
	char UnknownData_1F0[0x30]; // 0x1f0(0x30)
	char SfxPool[0x8]; // 0x220(0x08)
	char UnknownData_228[0x40]; // 0x228(0x40)
	bool ShowCapacityInDescription; // 0x268(0x01)
	char UnknownData_269[0x9f]; // 0x269(0x9f)
	char Aggregator[0x8]; // 0x308(0x08)

	//void TakeItem(struct AActor* Player, int32_t NodeIndex); // Function Athena.StorageContainerComponent.TakeItem // Final|Native|Public|BlueprintCallable // @ game+0x38b5810
	//void OnRep_QuickTakers(struct TArray<struct AActor*> InOldTakers); // Function Athena.StorageContainerComponent.OnRep_QuickTakers // Final|Native|Private // @ game+0x38b5560
	//void OnRep_QuickGivers(struct TArray<struct AActor*> InOldGivers); // Function Athena.StorageContainerComponent.OnRep_QuickGivers // Final|Native|Private // @ game+0x38b5460
	//void OnRep_ContentsChanged(struct FStorageContainerBackingStore InOldItemCount); // Function Athena.StorageContainerComponent.OnRep_ContentsChanged // Final|Native|Private // @ game+0x38b52d0
	//void Multicast_DetachAllPlayersRPC(); // Function Athena.StorageContainerComponent.Multicast_DetachAllPlayersRPC // Final|Net|NetReliableNative|Event|NetMulticast|Private // @ game+0x38b52b0
	//struct FText GetContainerDisplayName(); // Function Athena.StorageContainerComponent.GetContainerDisplayName // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x38b5220
	//void AddItem(struct AActor* Player, struct UClass* InItemDesc); // Function Athena.StorageContainerComponent.AddItem // Final|Native|Public|BlueprintCallable // @ game+0x38b5010
};


struct FFloatRange {
	float pad1;
	float min;
	float pad2;
	float max;
};

// STRIPED FROM Class Athena.Cannon
struct ACannonLoadedItemInfo
{
	char pad[0x768];
	struct AItemInfo* LoadedItemInfo; // 0x768(0x08)
};

struct ACannonSimple {
	char pad_0[0x5b4];
	float ProjectileSpeed; // 0x5b4(0x04)
	float ProjectileGravityScale; // 0x5b8(0x04)
	struct FFloatRange PitchRange; // 0x5bc(0x10)
	struct FFloatRange YawRange; // 0x5cc(0x10)
	char pad_1[0x1d4];
	float ServerPitch; // 0x7b0(0x04)
	float ServerYaw; // 0x7b4(0x04)
};
// Class Athena.Cannon
// Size: 0xcc0 (Inherited: 0x510)
struct ACannon {
	char pad_0[0x5b4];
	float ProjectileSpeed; // 0x5b4(0x04)
	float ProjectileGravityScale; // 0x5b8(0x04)
	struct FFloatRange PitchRange; // 0x5bc(0x10)
	struct FFloatRange YawRange; // 0x5cc(0x10)
	char pad_1[0x1d4];
	float ServerPitch; // 0x7b0(0x04)
	float ServerYaw; // 0x7b4(0x04)

	void HandlePitchInput(float Pitch)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.HandlePitchInput");
		struct {
			float Pitch;
		} params;
		params.Pitch = Pitch;

		ProcessEvent(this, fn, &params);
	}

	void HandleYawInput(float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.HandleYawInput");
		struct {
			float Yaw;
		} params;
		params.Yaw = Yaw;

		ProcessEvent(this, fn, &params);
	}

	void ForceAimCannon(float Pitch, float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.ForceAimCannon");
		struct {
			float Pitch;
			float Yaw;
		} params;
		params.Pitch = Pitch;
		params.Yaw = Yaw;
		ProcessEvent(this, fn, &params);
	}

	bool IsReadyToFire() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.IsReadyToFire");
		bool is_ready = true;
		ProcessEvent(this, fn, &is_ready);
		return is_ready;
	}

	void Fire() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Fire");
		ProcessEvent(this, fn, nullptr);
	}

	bool IsReadyToReload() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.IsReadyToReload");
		bool is_ready = true;
		ProcessEvent(this, fn, &is_ready);
		return is_ready;
	}

	void Reload() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Reload");
		ProcessEvent(this, fn, nullptr);
	}

	void Server_Fire(float Pitch, float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Server_Fire");
		struct {
			float Pitch;
			float Yaw;
		} params;
		params.Pitch = Pitch;
		params.Yaw = Yaw;
		ProcessEvent(this, fn, &params);
	}

};

struct ASlidingDoor {
	char pad_0x0[0x52C];
	FVector InitialDoorMeshLocation; // 0x052C

	void OpenDoor() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.SkeletonFortDoor.OpenDoor");
		ProcessEvent(this, fn, nullptr);
	}
};

struct APuzzleVault {
	char pad[0x1000];
	ASlidingDoor* OuterDoor; // 0x1000
};

struct FXMarksTheSpotMapMark {
	struct FVector2D Position; // 0x00(0x08)
	float Rotation; // 0x08(0x04)
	bool IsUnderground; // 0x0c(0x01)
	char UnknownData_D[0x3]; // 0x0d(0x03)
};

struct AXMarksTheSpotMap
{
	char pad1[0x0808];
	//struct FString                                     MapTexturePath;                                            // 0x0818(0x0010) (Net, ZeroConstructor, RepNotify, HasGetValueTypeHash)
	//char pad2[0x80];
	//TArray<struct FXMarksTheSpotMapMark>               Marks;                                                     // 0x08A8(0x0010) (Net, ZeroConstructor, RepNotify)
	//char pad3[0x18];
	//float                                              Rotation;                                                  // 0x08D0(0x0004) (Net, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)


	struct FString MapTexturePath; // 0x808(0x10)
	char MapInventoryTexturePath[0x10]; // 0x818(0x10)
	char UnknownData_828[0x70]; // 0x828(0x70)
	struct TArray<struct FXMarksTheSpotMapMark> Marks; // 0x898(0x10)
	char UnknownData_8A8[0x18]; // 0x8a8(0x18)
	float Rotation; // 0x8c0(0x04)
};

struct UWieldedItemComponent {
	char pad[0x02F0];
	ACharacter* CurrentlyWieldedItem; // 0x02F0
};


// ScriptStruct Athena.AthenaCharacterSwimParams
// Size: 0x9c (Inherited: 0x00)
struct FAthenaCharacterSwimParams {
	float EnterSwimmingDepth; // 0x00(0x04)
	float ExitSwimmingDepth; // 0x04(0x04)
	float FloatHeight; // 0x08(0x04)
	float ThirdPersonMeshOffset; // 0x0c(0x04)
	float ThirdPersonMeshAdjustTime; // 0x10(0x04)
	float FullGravityHeight; // 0x14(0x04)
	float SurfaceSwimDepth; // 0x18(0x04)
	float SurfaceSwimExitDepth; // 0x1c(0x04)
	float SubmergedBuoyancyDepth; // 0x20(0x04)
	float ZeroDragDepth; // 0x24(0x04)
	float MaxBuoyancyAcceleration; // 0x28(0x04)
	float SubmergedBuoyancyAcceleration; // 0x2c(0x04)
	float SubmergedBuoyancyTime; // 0x30(0x04)
	float DragFactor; // 0x34(0x04)
	float MaxDragAcceleration; // 0x38(0x04)
	float MinPushDownRoofAngle; // 0x3c(0x04)
	float MinPushDownRoofZ; // 0x40(0x04)
	float MinCorrectionVelForSurfaceSwim; // 0x44(0x04)
	float SurfaceVelBlendTime; // 0x48(0x04)
	float UnderwaterSwimmingEntryPitch; // 0x4c(0x04)
	float UnderwaterSwimmingEntryAccel; // 0x50(0x04)
	float UnderwaterSwimDepth; // 0x54(0x04)
	float UnderwaterSwimmingDragSpeed; // 0x58(0x04)
	float UnderwaterSwimmingSpeed; // 0x5c(0x04)
	float PitchBlendOffRate; // 0x60(0x04)
	float MaxSwimDepth; // 0x64(0x04)
	float MaxSwimDepthBuoyancy; // 0x68(0x04)
	float MaxSwimDepthBuoyancyRange; // 0x6c(0x04)
	float MaxSwimDepthDragFactor; // 0x70(0x04)
	float CharacterCentreOffset; // 0x74(0x04)
	char WaterDepthTraceQueryType; // 0x78(0x01)
	char UnknownData_79[0x3]; // 0x79(0x03)
	char SurfaceSwimSpeeds[0x0c]; // 0x7c(0x0c)
	char UnderwaterSwimSpeeds[0x0c]; // 0x88(0x0c)
	float SurfaceSwimmingDepthWhenEnteredWater; // 0x94(0x04)
	char UnknownData_98[0x4]; // 0x98(0x04)
};

// Class Athena.AthenaCharacterMovementComponent
// Size: 0x830 (Inherited: 0x590)
struct UAthenaCharacterMovementComponent {
	char pad_00[0x590];
	char UnknownData_590[0x8]; // 0x590(0x08)
	struct FAthenaCharacterSwimParams SwimParams; // 0x598(0x9c)
	float SprintSpdAmp; // 0x634(0x04)
	float SprintAccelAmp; // 0x638(0x04)
	float LookAtYawRate; // 0x63c(0x04)
	float LookAtPitchRate; // 0x640(0x04)
	char UnknownData_644[0x4]; // 0x644(0x04)
	float MaxSpeedRatioWhenWalkingInNonSwimWater; // 0x648(0x04)
	float MaxAccelerationRatioWhenWalkingInNonSwimWater; // 0x64c(0x04)
	char bCharacterCollisionSweepsEnabled : 1; // 0x650(0x01)
	char UnknownData_650_1 : 7; // 0x650(0x01)
	char UnknownData_651[0x3]; // 0x651(0x03)
	float CharacterCollisionRadius; // 0x654(0x04)
	char CharacterCollisionProfile[0x8]; // 0x658(0x08)
	char CollisionPanicFallbackProfileName[0x8]; // 0x660(0x08)
	float CharacterCollisionWalkingFriction; // 0x668(0x04)
	float CharacterCollisionSwimmingFriction; // 0x66c(0x04)
	float CharacterCollisionUnderwaterOffset; // 0x670(0x04)
	char UnknownData_674[0x4]; // 0x674(0x04)
	char UnderwaterMovement[0x8]; // 0x678(0x08)
	bool bCreateDisturbance; // 0x680(0x01)
	char UnknownData_681[0x3]; // 0x681(0x03)
	float DisturbanceSize; // 0x684(0x04)
	float DisturbanceVelocityScale; // 0x688(0x04)
	bool UseAsRVOObstacleOnly; // 0x68c(0x01)
	char UnknownData_68D[0x63]; // 0x68d(0x63)
	float FanPushOutAngle; // 0x6f0(0x04)
	int32_t FanPushOutNumSteps; // 0x6f4(0x04)
	float MaxFanPushOutScalar; // 0x6f8(0x04)
	char UnknownData_6FC[0x4]; // 0x6fc(0x04)
	char UnderwaterVelocityToAnglePitchProjectileHitVolume[0x8]; // 0x700(0x08)
	char UnderwaterVelocityToAngleRollProjectileHitVolume[0x8]; // 0x708(0x08)
	char UnderwaterVelocityToZOffsetProjectileHitVolume[0x8]; // 0x710(0x08)
	char OnLandVelocityToAnglePitchProjectileHitVolume[0x8]; // 0x718(0x08)
	char OnLandVelocityToXOffsetProjectileHitVolume[0x8]; // 0x720(0x08)
	char OnLandVelocityToYOffsetProjectileHitVolume[0x8]; // 0x728(0x08)
	char OnLandVelocityToZOffsetProjectileHitVolume[0x8]; // 0x730(0x08)
	char FallingVelocityToAnglePitchProjectileHitVolume[0x8]; // 0x738(0x08)
	char FallingVelocityToXOffsetProjectileHitVolume[0x8]; // 0x740(0x08)
	char FallingVelocityToZOffsetProjectileHitVolume[0x8]; // 0x748(0x08)
	char UnknownData_750[0xb0]; // 0x750(0xb0)
	struct FName NonUnderwaterCollisionProfileName; // 0x800(0x08)
	struct FName UnderwaterCollisionProfileName; // 0x808(0x08)
	char UnknownData_810[0x20]; // 0x810(0x20)

	void UnforceSwimmingClientTrustThreshold(); // Function Athena.AthenaCharacterMovementComponent.UnforceSwimmingClientTrustThreshold // Final|Native|Public|BlueprintCallable // @ game+0xc93e20
	void SetMovementMode(char NewMovementMode, char NewCustomMode); // Function Athena.AthenaCharacterMovementComponent.SetMovementMode // Native|Public|BlueprintCallable // @ game+0xc929e0
	void SetErrorOnResolvePenetration(bool ErrorOnResolve); // Function Athena.AthenaCharacterMovementComponent.SetErrorOnResolvePenetration // Final|Native|Public|BlueprintCallable // @ game+0xc922f0
	bool HasValidAckedMoveClientOnly(); // Function Athena.AthenaCharacterMovementComponent.HasValidAckedMoveClientOnly // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xc8f170
	float GetTimestampOfMostRecentPredictedMoveClientOnly(); // Function Athena.AthenaCharacterMovementComponent.GetTimestampOfMostRecentPredictedMoveClientOnly // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xc8e960
	float GetTimestampOfLastAckedMoveClientOnly(); // Function Athena.AthenaCharacterMovementComponent.GetTimestampOfLastAckedMoveClientOnly // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xc8e930
	float GetTerminalVelocity(); // Function Athena.AthenaCharacterMovementComponent.GetTerminalVelocity // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xc8e8c0
	char GetMovementMode(); // Function Athena.AthenaCharacterMovementComponent.GetMovementMode // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xc8e2d0
	float GetMaxSprintSpeed(); // Function Athena.AthenaCharacterMovementComponent.GetMaxSprintSpeed // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xc8e220
	float GetMaxMoveSpeedScalar(); // Function Athena.AthenaCharacterMovementComponent.GetMaxMoveSpeedScalar // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xc8e1f0
	void ForceSwimmingClientTrustThreshold(float NewSwimmingClientTrustThreshold); // Function Athena.AthenaCharacterMovementComponent.ForceSwimmingClientTrustThreshold // Final|Native|Public|BlueprintCallable // @ game+0xc8b300
	void FindCurrentFloor(); // Function Athena.AthenaCharacterMovementComponent.FindCurrentFloor // Final|Native|Public|BlueprintCallable // @ game+0xc8b000
};
// ScriptStruct ObjectMessaging.ObjectMessagingDispatcher
// Size: 0xa0 (Inherited: 0x00)
struct FObjectMessagingDispatcher {
	char UnknownData_0[0xa0]; // 0x00(0xa0)
};

// ScriptStruct Animation.Docker
// Size: 0x90 (Inherited: 0x00)
struct FDocker {
	char UnknownData_0[0x90]; // 0x00(0x90)
};

// Class Athena.AthenaCharacter   AAthenaCharacter -> AthenaCharacterMovementComponent*(0x718) -> SwimParams(0x568) -> EnterSwimmingDepth(0x0)
// Size: 0xbb0 (Inherited: 0x5e0)
struct AAthenaCharacter : ACharacter {
		char UnknownData_5E0[0x120]; // 0x5e0(0x120)
		struct ULookAtOffsetComponent* LookAtOffsetComponent; // 0x700(0x08)
		struct UAnimationDataStoreAsset* AnimationDataStoreAsset; // 0x708(0x08)
		struct UAthenaAnimationSittingDataAsset* SittingAnimationDataAssets; // 0x710(0x08)
		struct UAthenaCharacterMovementComponent* CharacterMovementComponent; // 0x718(0x08)
		struct FDocker Docker; // 0x720(0x90)
		struct UActionStateMachineComponent* ActionStateMachineComponent; // 0x7b0(0x08)
		struct UActionStatePriorityTableData* ActionStatePriorityTableData; // 0x7b8(0x08)
		struct UClass* ActionStateCreatorDefinition; // 0x7c0(0x08)
		struct FObjectMessagingDispatcher ObjectMessageDispatcher; // 0x7c8(0xa0)
		struct UWieldableItemSetComponent* WieldableItemSetComponent; // 0x868(0x08)
		struct UWieldedItemComponent* WieldedItemComponent; // 0x870(0x08)
		struct USceneComponent* MeshAdjustmentComponent; // 0x878(0x08)
		struct UInventoryManipulatorComponent* InventoryManipulatorComponent; // 0x880(0x08)
		struct ULoadoutComponent* LoadoutComponent; // 0x888(0x08)
		struct UItemLoadoutComponent* ItemLoadoutComponent; // 0x890(0x08)
		struct UHealthComponent* HealthComponent; // 0x898(0x08)
		struct UAthenaCharacterStateManagerComponent* StateManagerComponent; // 0x8a0(0x08)
		struct URewindComponent* RewindComponent; // 0x8a8(0x08)
		struct UActorDamageableComponent* ActorDamageableComponent; // 0x8b0(0x08)
		struct UCharacterHitReactionComponent* CharacterHitReactionComponent; // 0x8b8(0x08)
		struct UAnimNotifyWwiseEmitterComponent* AnimNotifyWwiseEmitterComponent; // 0x8c0(0x08)
		struct UMeleeAttackProcessorComponent* MeleeAttackProcessor; // 0x8c8(0x08)
		struct UHitDetectionComponent* HitDetectionComponent; // 0x8d0(0x08)
		struct UAthenaCharacterKnockbackComponent* KnockBackComponent; // 0x8d8(0x08)
		struct UAthenaCharacterDodgeComponent* DodgeComponent; // 0x8e0(0x08)
		struct UCharacterAINoiseEventComponent* CharacterAINoiseEventComponent; // 0x8e8(0x08)
		struct UActionRulesComponent* PrivateActionRulesComponent; // 0x8f0(0x08)
		struct UShipTrackerComponent* ShipTrackerComponent; // 0x8f8(0x08)
		struct UWatercraftTrackerComponent* WatercraftTrackerComponent; // 0x900(0x08)
		struct UItemLoaderComponent* ItemLoaderComponent; // 0x908(0x08)
		struct URetractableCharacterComponent* RetractableCharacterComponent; // 0x910(0x08)
		struct UResourceProviderComponent* ResourceProviderComponent; // 0x918(0x08)
		struct UCharacterFlammableComponent* CharacterFlammableComponent; // 0x920(0x08)
		struct USoakableComponent* SoakableComponent; // 0x928(0x08)
		struct UWaterExposureComponent* WaterExposureComponent; // 0x930(0x08)
		struct UShelteredComponent* ShelteredComponent; // 0x938(0x08)
		struct UBoxedRpcDispatcherComponent* BoxedRpcDispatcherComponent; // 0x940(0x08)
		struct UCosmeticItemAnimationComponent* CosmeticItemAnimationComponent; // 0x948(0x08)
		char pad0[0x2A0];
		struct USkeletalMeshComponent* GetThirdPersonMeshComponent()
		{
			static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.GetThirdPersonMeshComponent");
			bool getthird;
			ProcessEvent(this, fn, &getthird);
		};
		struct USkeletalMeshComponent* GetFirstPersonMeshComponent()
		{
			static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.GetFirstPersonMeshComponent");
			bool getfirst;
			ProcessEvent(this, fn, &getfirst);
		};
};
struct UHealthComponent {
	float GetMaxHealth() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HealthComponent.GetMaxHealth");
		float health = 0.f;
		ProcessEvent(this, fn, &health);
		return health;
	}
	float GetCurrentHealth() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HealthComponent.GetCurrentHealth");
		float health = 0.f;
		ProcessEvent(this, fn, &health);
		return health;
	};
};
// Class Athena.AthenaPlayerCharacter
// Size: 0x1bf0 (Inherited: 0xbb0)
struct AAthenaPlayerCharacter : AAthenaCharacter {
	char pad[0x198];
	struct UDrowningComponent* DrowningComponent; // 0xd48(0x08)
	char pad2[0xEA0];

	ACharacter* GetWieldedItem() {
		if (!WieldedItemComponent) return nullptr;
		return WieldedItemComponent->CurrentlyWieldedItem;
	}

};

// Class Athena.DrowningComponent
// Size: 0x1c0 (Inherited: 0xc8)
struct UDrowningComponent {
	char pad[0x108];
	float OxygenLevel; // 0x108(0x04)
	char pad2[0xB4];

	float GetOxygenLevel() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.DrowningComponent.GetOxygenLevel");
		float oxygen;
		ProcessEvent(this, fn, &oxygen);
		return oxygen;
	} // Function Athena.DrowningComponent.GetOxygenLevel // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x39cb270
};

struct AGameCustomizationService_SetTime_Params
{
	int Hours;

	void SetTime(int Hours)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.GameCustomizationService.SetTime");
		AGameCustomizationService_SetTime_Params params;
		params.Hours = Hours;
		ProcessEvent(this, fn, &params);
	}
};

struct FMinimalViewInfo {
	FVector Location;
	FRotator Rotation;
	char UnknownData_18[0x10];
	//float FOV;
};

struct FCameraCacheEntry {
	float TimeStamp;
	char pad[0xc];
	FMinimalViewInfo POV;
};

struct FTViewTarget
{
	class AActor* Target;                                                    // 0x0000(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	unsigned char                                      UnknownData_KHBN[0x8];                                     // 0x0008(0x0008) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	struct FMinimalViewInfo                            POV;                                                       // 0x0010(0x05A0) (Edit, BlueprintVisible)
	class APlayerState* PlayerState;                                               // 0x05B0(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	unsigned char                                      UnknownData_QIMB[0x8];                                     // 0x05B8(0x0008) MISSED OFFSET (PADDING)

};

struct APlayerCameraManager {
	char pad[0x0440];
	FCameraCacheEntry CameraCache; // 0x0440
	FCameraCacheEntry LastFrameCameraCache;
	FTViewTarget ViewTarget;
	FTViewTarget PendingViewTarget;


	FVector GetCameraLocation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerCameraManager.GetCameraLocation");
		FVector location;
		ProcessEvent(this, fn, &location);
		return location;
	};
	FRotator GetCameraRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerCameraManager.GetCameraRotation");
		FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}
};

struct FKey
{
	FName KeyName;
	unsigned char UnknownData00[0x18] = {};

	FKey() {};
	FKey(const char* InName) : KeyName(FName(InName)) {}
};

struct AController_K2_GetPawn_Params
{
	class APawn* ReturnValue;
};

struct AController {

	char pad_0000[0x03E0];
	class ACharacter* Character; //0x03E0
	char pad_0480[0x70];
	APlayerCameraManager* PlayerCameraManager; //0x0458
	char pad_04f8[0x1031];
	bool IdleDisconnectEnabled; // 0x14D1

	void UnPossess()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.UnPossess");
		ProcessEvent(this, fn, nullptr);
	}

	void SetTime(int Hours)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.GameCustomizationService.SetTime");
		AGameCustomizationService_SetTime_Params params;
		params.Hours = Hours;
		ProcessEvent(this, fn, &params);
	}

	void SendToConsole(FString& cmd) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.SendToConsole");
		ProcessEvent(this, fn, &cmd);
	}

	bool WasInputKeyJustPressed(const FKey& Key) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.WasInputKeyJustPressed");
		struct
		{
			FKey Key;

			bool ReturnValue = false;
		} params;

		params.Key = Key;
		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	APawn* K2_GetPawn() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.K2_GetPawn");
		AController_K2_GetPawn_Params params;
		class APawn* ReturnValue;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	bool ProjectWorldLocationToScreen(const FVector& WorldLocation, FVector2D& ScreenLocation) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.ProjectWorldLocationToScreen");
		struct
		{
			FVector WorldLocation;
			FVector2D ScreenLocation;
			bool ReturnValue = false;
		} params;

		params.WorldLocation = WorldLocation;
		ProcessEvent(this, fn, &params);
		ScreenLocation = params.ScreenLocation;
		return params.ReturnValue;
	}

	FRotator GetControlRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Pawn.GetControlRotation");
		struct FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}

	FRotator GetDesiredRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Pawn.GetDesiredRotation");
		struct FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}

	void AddYawInput(float Val) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.AddYawInput");
		ProcessEvent(this, fn, &Val);
	}

	void AddPitchInput(float Val) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.AddPitchInput");
		ProcessEvent(this, fn, &Val);
	}

	void FOV(float NewFOV) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.FOV");
		ProcessEvent(this, fn, &NewFOV);
	}

	bool LineOfSightTo(ACharacter* Other, const FVector& ViewPoint, const bool bAlternateChecks) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.LineOfSightTo");
		struct {
			ACharacter* Other = nullptr;
			FVector ViewPoint;
			bool bAlternateChecks = false;
			bool ReturnValue = false;
		} params;
		params.Other = Other;
		params.ViewPoint = ViewPoint;
		params.bAlternateChecks = bAlternateChecks;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}
};
// ScriptStruct Engine.ViewTargetTransitionParams
// Size: 0x10 (Inherited: 0x00)
struct FViewTargetTransitionParams {
	float BlendTime; // 0x00(0x04)
	char BlendFunction; // 0x04(0x01)
	char UnknownData_5[0x3]; // 0x05(0x03)
	float BlendExp; // 0x08(0x04)
	char bLockOutgoing : 1; // 0x0c(0x01)
	char UnknownData_C_1 : 7; // 0x0c(0x01)
	char UnknownData_D[0x3]; // 0x0d(0x03)
};
// Class Engine.PlayerController
// Size: 0x728 (Inherited: 0x430)
struct APlayerController : AController {
	struct UPlayer* Player; // 0x430(0x08)
	char UnknownData_438[0x8]; // 0x438(0x08)
	struct APawn* AcknowledgedPawn; // 0x440(0x08)
	struct UInterpTrackInstDirector* ControllingDirTrackInst; // 0x448(0x08)
	char UnknownData_450[0x8]; // 0x450(0x08)
	struct AHUD* MyHUD; // 0x458(0x08)
	struct APlayerCameraManager* PlayerCameraManager; // 0x460(0x08)
	struct UClass* PlayerCameraManagerClass; // 0x468(0x08)
	bool bAutoManageActiveCameraTarget; // 0x470(0x01)
	char UnknownData_471[0x3]; // 0x471(0x03)
	struct FRotator TargetViewRotation; // 0x474(0x0c)
	char UnknownData_480[0x10]; // 0x480(0x10)
	struct TArray<struct AActor*> HiddenActors; // 0x490(0x10)
	float LastSpectatorStateSynchTime; // 0x4a0(0x04)
	struct FVector LastSpectatorSyncLocation; // 0x4a4(0x0c)
	struct FRotator LastSpectatorSyncRotation; // 0x4b0(0x0c)
	int32_t ClientCap; // 0x4bc(0x04)
	struct UCheatManager* CheatManager; // 0x4c0(0x08)
	struct UClass* CheatClass; // 0x4c8(0x08)
	struct UPlayerInput* PlayerInput; // 0x4d0(0x08)
	struct TArray<struct FActiveForceFeedbackEffect> ActiveForceFeedbackEffects; // 0x4d8(0x10)
	char UnknownData_4E8[0x90]; // 0x4e8(0x90)
	char UnknownData_578_0 : 2; // 0x578(0x01)
	char bPlayerIsWaiting : 1; // 0x578(0x01)
	char UnknownData_578_3 : 5; // 0x578(0x01)
	char UnknownData_579[0x3]; // 0x579(0x03)
	char NetPlayerIndex; // 0x57c(0x01)
	bool bHasVoiceHandshakeCompleted; // 0x57d(0x01)
	char UnknownData_57E[0x2]; // 0x57e(0x02)
	struct UNetConnection* PendingSwapConnection; // 0x580(0x08)
	struct UNetConnection* NetConnection; // 0x588(0x08)
	char UnknownData_590[0xc]; // 0x590(0x0c)
	float InputYawScale; // 0x59c(0x04)
	float InputPitchScale; // 0x5a0(0x04)
	float InputRollScale; // 0x5a4(0x04)
	char bShowMouseCursor : 1; // 0x5a8(0x01)
	char bEnableClickEvents : 1; // 0x5a8(0x01)
	char bEnableTouchEvents : 1; // 0x5a8(0x01)
	char bEnableMouseOverEvents : 1; // 0x5a8(0x01)
	char bEnableTouchOverEvents : 1; // 0x5a8(0x01)
	char bForceFeedbackEnabled : 1; // 0x5a8(0x01)
	char UnknownData_5A8_6 : 2; // 0x5a8(0x01)
	char UnknownData_5A9[0x3]; // 0x5a9(0x03)
	char DefaultMouseCursor; // 0x5ac(0x01)
	char CurrentMouseCursor; // 0x5ad(0x01)
	char DefaultClickTraceChannel; // 0x5ae(0x01)
	char CurrentClickTraceChannel; // 0x5af(0x01)
	float HitResultTraceDistance; // 0x5b0(0x04)
	float ForceFeedbackIntensity; // 0x5b4(0x04)
	char UnknownData_5B8[0x88]; // 0x5b8(0x88)
	struct UInputComponent* InactiveStateInputComponent; // 0x640(0x08)
	char UnknownData_648[0x20]; // 0x648(0x20)
	struct UTouchInterface* CurrentTouchInterface; // 0x668(0x08)
	char UnknownData_670[0x30]; // 0x670(0x30)
	struct ASpectatorPawn* SpectatorPawn; // 0x6a0(0x08)
	struct FVector SpawnLocation; // 0x6a8(0x0c)
	char UnknownData_6B4[0x4]; // 0x6b4(0x04)
	char UnknownData_6B8[0x50]; // 0x6b8(0x50)
	char UnknownData_708[0x14]; // 0x708(0x14)
	bool bIsLocalPlayerController; // 0x71c(0x01)
	char UnknownData_71D[0x1]; // 0x71d(0x01)
	uint16_t SeamlessTravelCount; // 0x71e(0x02)
	uint16_t LastCompletedSeamlessTravelCount; // 0x720(0x02)
	bool bInCinematicMode; // 0x722(0x01)
	char UnknownData_723[0x5]; // 0x723(0x05)

	bool WasInputKeyJustReleased(struct FKey Key); // Function Engine.PlayerController.WasInputKeyJustReleased // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b33810
	bool WasInputKeyJustPressed(struct FKey Key); // Function Engine.PlayerController.WasInputKeyJustPressed // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b33710
	void ToggleSpeaking(bool bInSpeaking); // Function Engine.PlayerController.ToggleSpeaking // Exec|Native|Public // @ game+0x2b33340
	void SwitchLevel(struct FString URL); // Function Engine.PlayerController.SwitchLevel // Exec|Native|Public // @ game+0x2b33250
	void StopHapticEffect(char Hand); // Function Engine.PlayerController.StopHapticEffect // Final|Native|Public|BlueprintCallable // @ game+0x2b331b0
	void StartFire(char FireModeNum); // Function Engine.PlayerController.StartFire // Exec|Native|Public // @ game+0x2b32d80
	void SetVirtualJoystickVisibility(bool bVisible); // Function Engine.PlayerController.SetVirtualJoystickVisibility // Native|Public|BlueprintCallable // @ game+0x2b326a0
	void SetViewTargetWithBlend(struct AActor* NewViewTarget, float BlendTime, char BlendFunc, float BlendExp, bool bLockOutgoing); // Function Engine.PlayerController.SetViewTargetWithBlend // Native|Public|BlueprintCallable // @ game+0x2b32520
	void SetName(struct FString S); // Function Engine.PlayerController.SetName // Exec|Native|Public // @ game+0x2b31ea0
	void SetIgnoreMoveInput(bool bNewMoveInput); // Function Engine.PlayerController.SetIgnoreMoveInput // Native|Public|BlueprintCallable // @ game+0x2b319e0
	void SetIgnoreLookInput(bool bNewLookInput); // Function Engine.PlayerController.SetIgnoreLookInput // Native|Public|BlueprintCallable // @ game+0x2b31950
	void SetHapticsByValue(float Frequency, float Amplitude, char Hand); // Function Engine.PlayerController.SetHapticsByValue // Final|Native|Public|BlueprintCallable // @ game+0x2b31770
	void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning); // Function Engine.PlayerController.SetCinematicMode // Native|Public|BlueprintCallable // @ game+0x2b31290
	void SetAudioListenerOverride(struct USceneComponent* AttachToComponent, struct FVector Location, struct FRotator Rotation); // Function Engine.PlayerController.SetAudioListenerOverride // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2b30ff0
	void ServerViewSelf(struct FViewTargetTransitionParams TransitionParams); // Function Engine.PlayerController.ServerViewSelf // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b30860
	void ServerViewPrevPlayer(); // Function Engine.PlayerController.ServerViewPrevPlayer // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b30810
	void ServerViewNextPlayer(); // Function Engine.PlayerController.ServerViewNextPlayer // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b307c0
	void ServerVerifyViewTarget(); // Function Engine.PlayerController.ServerVerifyViewTarget // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30770
	void ServerUpdateCamera(struct FVector_NetQuantize CamLoc, int32_t CamPitchAndYaw); // Function Engine.PlayerController.ServerUpdateCamera // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b30650
	void ServerUnmutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ServerUnmutePlayer // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30500
	void ServerToggleAILogging(); // Function Engine.PlayerController.ServerToggleAILogging // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b304b0
	void ServerShortTimeout(); // Function Engine.PlayerController.ServerShortTimeout // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30460
	void ServerSetSpectatorWaiting(bool bWaiting); // Function Engine.PlayerController.ServerSetSpectatorWaiting // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b303a0
	void ServerSetSpectatorLocation(struct FVector NewLoc, struct FRotator NewRot); // Function Engine.PlayerController.ServerSetSpectatorLocation // Net|Native|Event|Public|NetServer|HasDefaults|NetValidate // @ game+0x2b30270
	void ServerRestartPlayer(); // Function Engine.PlayerController.ServerRestartPlayer // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30220
	void ServerPause(); // Function Engine.PlayerController.ServerPause // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b301d0
	void ServerNotifyLoadedWorld(struct FName WorldPackageName); // Function Engine.PlayerController.ServerNotifyLoadedWorld // Final|Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30120
	void ServerMutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ServerMutePlayer // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2ffd0
	void ServerCheckClientPossessionReliable(); // Function Engine.PlayerController.ServerCheckClientPossessionReliable // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2ff80
	void ServerCheckClientPossession(); // Function Engine.PlayerController.ServerCheckClientPossession // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b2ff30
	void ServerChangeName(struct FString S); // Function Engine.PlayerController.ServerChangeName // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2fe60
	void ServerCamera(struct FName NewMode); // Function Engine.PlayerController.ServerCamera // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2fda0
	void ServerAcknowledgePossession(struct APawn* P); // Function Engine.PlayerController.ServerAcknowledgePossession // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2fce0
	void SendToConsole(struct FString Command); // Function Engine.PlayerController.SendToConsole // Exec|Native|Public // @ game+0x2b2fc30
	void RestartLevel(); // Function Engine.PlayerController.RestartLevel // Exec|Native|Public // @ game+0x2b2fc10
	void ResetIgnoreMoveInput(); // Function Engine.PlayerController.ResetIgnoreMoveInput // Native|Public|BlueprintCallable // @ game+0x2b2fbd0
	void ResetIgnoreLookInput(); // Function Engine.PlayerController.ResetIgnoreLookInput // Native|Public|BlueprintCallable // @ game+0x2b2fbb0
	void ResetIgnoreInputFlags(); // Function Engine.PlayerController.ResetIgnoreInputFlags // Native|Public|BlueprintCallable // @ game+0x2b2fb90
	bool ProjectWorldLocationToScreen(const FVector& WorldLocation, FVector2D& ScreenLocation) // Function Engine.PlayerController.ProjectWorldLocationToScreen // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2f450
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.ProjectWorldLocationToScreen");
		struct
		{
			FVector WorldLocation;
			FVector2D ScreenLocation;
			bool ReturnValue = false;
		} params;

		params.WorldLocation = WorldLocation;
		ProcessEvent(this, fn, &params);
		ScreenLocation = params.ScreenLocation;
		return params.ReturnValue;
	}
	void PlayHapticEffect(struct UHapticFeedbackEffect* HapticEffect, char Hand, float Scale); // Function Engine.PlayerController.PlayHapticEffect // Final|Native|Public|BlueprintCallable // @ game+0x2b2f2b0
	void PlayDynamicForceFeedback(float Intensity, float Duration, bool bAffectsLeftLarge, bool bAffectsLeftSmall, bool bAffectsRightLarge, bool bAffectsRightSmall, char Action, struct FLatentActionInfo LatentInfo); // Function Engine.PlayerController.PlayDynamicForceFeedback // Final|Native|Public|BlueprintCallable // @ game+0x2b2f020
	void Pause(); // Function Engine.PlayerController.Pause // Exec|Native|Public // @ game+0x2b2eaf0
	void OnServerStartedVisualLogger(bool bIsLogging); // Function Engine.PlayerController.OnServerStartedVisualLogger // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b2ea60
	void LocalTravel(struct FString URL); // Function Engine.PlayerController.LocalTravel // Exec|Native|Public // @ game+0x2b2e5e0
	bool IsSeamlessTravelInProgress(); // Function Engine.PlayerController.IsSeamlessTravelInProgress // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a930
	bool IsPossessingServerAcknowledgedPawn(); // Function Engine.PlayerController.IsPossessingServerAcknowledgedPawn // Final|Native|Public|BlueprintCallable|BlueprintPure // @ game+0x2b2a8d0
	bool IsMoveInputIgnored(); // Function Engine.PlayerController.IsMoveInputIgnored // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a810
	bool IsLookInputIgnored(); // Function Engine.PlayerController.IsLookInputIgnored // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a7e0
	bool IsInputKeyDown(struct FKey Key); // Function Engine.PlayerController.IsInputKeyDown // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a680
	void GetViewportSize(int32_t SizeX, int32_t SizeY); // Function Engine.PlayerController.GetViewportSize // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a290
	struct ASpectatorPawn* GetSpectatorPawn(); // Function Engine.PlayerController.GetSpectatorPawn // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b29d20
	bool GetMousePosition(float LocationX, float LocationY); // Function Engine.PlayerController.GetMousePosition // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b29500
	struct FVector GetInputVectorKeyState(struct FKey Key); // Function Engine.PlayerController.GetInputVectorKeyState // Final|Native|Public|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b292b0
	void GetInputTouchState(char FingerIndex, float LocationX, float LocationY, bool bIsCurrentlyPressed); // Function Engine.PlayerController.GetInputTouchState // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b29010
	void GetInputMouseDelta(float DeltaX, float DeltaY); // Function Engine.PlayerController.GetInputMouseDelta // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28f30
	void GetInputMotionState(struct FVector Tilt, struct FVector RotationRate, struct FVector Gravity, struct FVector Acceleration); // Function Engine.PlayerController.GetInputMotionState // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28da0
	float GetInputKeyTimeDown(struct FKey Key); // Function Engine.PlayerController.GetInputKeyTimeDown // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28ca0
	void GetInputAnalogStickState(char WhichStick, float StickX, float StickY); // Function Engine.PlayerController.GetInputAnalogStickState // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b289e0
	float GetInputAnalogKeyState(struct FKey Key); // Function Engine.PlayerController.GetInputAnalogKeyState // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b288e0
	struct AHUD* GetHUD(); // Function Engine.PlayerController.GetHUD // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27e10
	bool GetHitResultUnderFingerForObjects(char FingerIndex, struct TArray<char> ObjectTypes, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderFingerForObjects // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b285d0
	bool GetHitResultUnderFingerByChannel(char FingerIndex, char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderFingerByChannel // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28430
	bool GetHitResultUnderFinger(char FingerIndex, char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderFinger // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28290
	bool GetHitResultUnderCursorForObjects(struct TArray<char> ObjectTypes, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderCursorForObjects // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28100
	bool GetHitResultUnderCursorByChannel(char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderCursorByChannel // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27fa0
	bool GetHitResultUnderCursor(char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderCursor // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27e40
	struct FVector GetFocalLocation(); // Function Engine.PlayerController.GetFocalLocation // Native|Public|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27d90
	void EnableCheats(); // Function Engine.PlayerController.EnableCheats // Exec|Native|Public // @ game+0x2b265a0
	bool DeprojectScreenPositionToWorld(float ScreenX, float ScreenY, struct FVector WorldLocation, struct FVector WorldDirection); // Function Engine.PlayerController.DeprojectScreenPositionToWorld // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b26190
	bool DeprojectMousePositionToWorld(struct FVector WorldLocation, struct FVector WorldDirection); // Function Engine.PlayerController.DeprojectMousePositionToWorld // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b26090
	void ConsoleKey(struct FKey Key); // Function Engine.PlayerController.ConsoleKey // Exec|Native|Public // @ game+0x2b25e50
	void ClientWasKicked(struct FText KickReason); // Function Engine.PlayerController.ClientWasKicked // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25d20
	void ClientVoiceHandshakeComplete(); // Function Engine.PlayerController.ClientVoiceHandshakeComplete // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25d00
	void ClientUpdateLevelStreamingStatusBatched(struct FString PackageBasePath, struct TArray<struct FLevelStreamingStatusUpdateInfo> LevelStreamingStatusUpdateInfo); // Function Engine.PlayerController.ClientUpdateLevelStreamingStatusBatched // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25bd0
	void ClientUpdateLevelStreamingStatus(struct FName PackageName, bool bNewShouldBeLoaded, bool bNewShouldBeVisible, bool bNewShouldBlockOnLoad, int32_t LODIndex); // Function Engine.PlayerController.ClientUpdateLevelStreamingStatus // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25a20
	void ClientUnmutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ClientUnmutePlayer // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25930
	void ClientTravelInternal(struct FString URL, char TravelType, bool bSeamless, struct FGuid MapPackageGuid); // Function Engine.PlayerController.ClientTravelInternal // Net|NetReliableNative|Event|Public|HasDefaults|NetClient // @ game+0x2b257b0
	void ClientTravel(struct FString URL, char TravelType, bool bSeamless, struct FGuid MapPackageGuid); // Function Engine.PlayerController.ClientTravel // Final|Native|Public|HasDefaults // @ game+0x2b25630
	void ClientTeamMessage(struct APlayerState* SenderPlayerState, struct FString S, struct FName Type, float MsgLifeTime); // Function Engine.PlayerController.ClientTeamMessage // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b254c0
	void ClientStopForceFeedback(struct UForceFeedbackEffect* ForceFeedbackEffect, struct FName Tag); // Function Engine.PlayerController.ClientStopForceFeedback // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b253f0
	void ClientStopCameraShake(struct UClass* Shake); // Function Engine.PlayerController.ClientStopCameraShake // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b25360
	void ClientStopCameraAnim(struct UCameraAnim* AnimToStop); // Function Engine.PlayerController.ClientStopCameraAnim // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b252d0
	void ClientStartOnlineSession(); // Function Engine.PlayerController.ClientStartOnlineSession // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b252b0
	void ClientSpawnCameraLensEffect(struct UClass* LensEffectEmitterClass); // Function Engine.PlayerController.ClientSpawnCameraLensEffect // Net|Native|Event|Public|NetClient|BlueprintCallable // @ game+0x2b25220
	void ClientSetViewTarget(struct AActor* A, struct FViewTargetTransitionParams TransitionParams)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.FOVHandlerFunctions.SetTargetFOV");
		struct
		{
			AActor* Character;
			FViewTargetTransitionParams ReturnValue;
		}params;
		A = params.Character;
		TransitionParams = params.ReturnValue;
		ProcessEvent(this, fn, &params);

	}; // Function Engine.PlayerController.ClientSetViewTarget // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25100
	void ClientSetSpectatorWaiting(bool bWaiting); // Function Engine.PlayerController.ClientSetSpectatorWaiting // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25070
	void ClientSetHUD(struct UClass* NewHUDClass); // Function Engine.PlayerController.ClientSetHUD // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b24e20
	void ClientSetForceMipLevelsToBeResident(struct UMaterialInterface* Material, float ForceDuration, int32_t CinematicTextureGroups); // Function Engine.PlayerController.ClientSetForceMipLevelsToBeResident // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24d20
	void ClientSetCinematicMode(bool bInCinematicMode, bool bAffectsMovement, bool bAffectsTurning, bool bAffectsHUD); // Function Engine.PlayerController.ClientSetCinematicMode // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24bb0
	void ClientSetCameraMode(struct FName NewCamMode); // Function Engine.PlayerController.ClientSetCameraMode // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24b20
	void ClientSetCameraFade(bool bEnableFading, struct FColor FadeColor, struct FVector2D FadeAlpha, float FadeTime, bool bFadeAudio); // Function Engine.PlayerController.ClientSetCameraFade // Net|NetReliableNative|Event|Public|HasDefaults|NetClient // @ game+0x2b24990
	void ClientSetBlockOnAsyncLoading(); // Function Engine.PlayerController.ClientSetBlockOnAsyncLoading // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24970
	void ClientSeamlessTravelComplete(); // Function Engine.PlayerController.ClientSeamlessTravelComplete // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24950
	void ClientReturnToMainMenu(struct FString ReturnReason); // Function Engine.PlayerController.ClientReturnToMainMenu // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b248a0
	void ClientRetryClientRestart(struct APawn* NewPawn); // Function Engine.PlayerController.ClientRetryClientRestart // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24810
	void ClientRestart(struct APawn* NewPawn); // Function Engine.PlayerController.ClientRestart // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24780
	void ClientReset(); // Function Engine.PlayerController.ClientReset // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24760
	void ClientRepObjRef(struct UObject* Object); // Function Engine.PlayerController.ClientRepObjRef // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b246d0
	void ClientReceiveLocalizedMessage(struct UClass* Message, int32_t Switch, struct APlayerState* RelatedPlayerState_2, struct APlayerState* RelatedPlayerState_3, struct UObject* OptionalObject); // Function Engine.PlayerController.ClientReceiveLocalizedMessage // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24550
	void ClientPrestreamTextures(struct AActor* ForcedActor, float ForceDuration, bool bEnableStreaming, int32_t CinematicTextureGroups); // Function Engine.PlayerController.ClientPrestreamTextures // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24400
	void ClientPrepareMapChange(struct FName LevelName, bool bFirst, bool bLast); // Function Engine.PlayerController.ClientPrepareMapChange // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b242f0
	void ClientPlaySoundAtLocation(struct USoundBase* Sound, struct FVector Location, float VolumeMultiplier, float PitchMultiplier); // Function Engine.PlayerController.ClientPlaySoundAtLocation // Net|Native|Event|Public|HasDefaults|NetClient // @ game+0x2b241a0
	void ClientPlaySound(struct USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier); // Function Engine.PlayerController.ClientPlaySound // Net|Native|Event|Public|NetClient // @ game+0x2b240a0
	void ClientPlayForceFeedback(struct UForceFeedbackEffect* ForceFeedbackEffect, bool bLooping, struct FName Tag); // Function Engine.PlayerController.ClientPlayForceFeedback // Net|Native|Event|Public|NetClient|BlueprintCallable // @ game+0x2b23f90
	void ClientPlayCameraShake(struct UClass* Shake, float Scale, char PlaySpace, struct FRotator UserPlaySpaceRot); // Function Engine.PlayerController.ClientPlayCameraShake // Net|Native|Event|Public|HasDefaults|NetClient|BlueprintCallable // @ game+0x2b23e30
	void ClientPlayCameraAnim(struct UCameraAnim* AnimToPlay, float Scale, float Rate, float BlendInTime, float BlendOutTime, bool bLoop, bool bRandomStartTime, char Space, struct FRotator CustomPlaySpace); // Function Engine.PlayerController.ClientPlayCameraAnim // Net|Native|Event|Public|HasDefaults|NetClient|BlueprintCallable // @ game+0x2b23b90
	void ClientMutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ClientMutePlayer // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23aa0
	void ClientMessage(struct FString S, struct FName Type, float MsgLifeTime); // Function Engine.PlayerController.ClientMessage // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23970
	void ClientIgnoreMoveInput(bool bIgnore); // Function Engine.PlayerController.ClientIgnoreMoveInput // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b238e0
	void ClientIgnoreLookInput(bool bIgnore); // Function Engine.PlayerController.ClientIgnoreLookInput // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23850
	void ClientGotoState(struct FName NewState); // Function Engine.PlayerController.ClientGotoState // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b237c0
	void ClientGameEnded(struct AActor* EndGameFocus, bool bIsWinner); // Function Engine.PlayerController.ClientGameEnded // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b236f0
	void ClientForceGarbageCollection(); // Function Engine.PlayerController.ClientForceGarbageCollection // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b236d0
	void ClientFlushLevelStreaming(); // Function Engine.PlayerController.ClientFlushLevelStreaming // Final|Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b236b0
	void ClientEndOnlineSession(); // Function Engine.PlayerController.ClientEndOnlineSession // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23690
	void ClientEnableNetworkVoice(bool bEnable); // Function Engine.PlayerController.ClientEnableNetworkVoice // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23600
	void ClientCommitMapChange(); // Function Engine.PlayerController.ClientCommitMapChange // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b235e0
	void ClientClearCameraLensEffects(); // Function Engine.PlayerController.ClientClearCameraLensEffects // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b235c0
	void ClientCapBandwidth(int32_t Cap); // Function Engine.PlayerController.ClientCapBandwidth // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23530
	void ClientCancelPendingMapChange(); // Function Engine.PlayerController.ClientCancelPendingMapChange // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23510
	void ClientBlendOutCameraShake(struct UClass* Shake); // Function Engine.PlayerController.ClientBlendOutCameraShake // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b23480
	void ClientAddTextureStreamingLoc(struct FVector InLoc, float Duration, bool bOverrideLocation); // Function Engine.PlayerController.ClientAddTextureStreamingLoc // Final|Net|NetReliableNative|Event|Public|HasDefaults|NetClient // @ game+0x2b23360
	void ClearAudioListenerOverride(); // Function Engine.PlayerController.ClearAudioListenerOverride // Final|Native|Public|BlueprintCallable // @ game+0x2b23320
	void Camera(struct FName NewMode); // Function Engine.PlayerController.Camera // Exec|Native|Public // @ game+0x2b23260
	void ActivateTouchInterface(struct UTouchInterface* NewTouchInterface); // Function Engine.PlayerController.ActivateTouchInterface // Native|Public|BlueprintCallable // @ game+0x2b22540
};

struct ULevel
{
	char UnkownData[0xa0];
	TArray<class ACharacter*> AActors; // 0x00A0(0x0010)
};
/*
// Class Engine.Level
// Size: 0x300 (Inherited: 0x28)
struct ULevel : UObject {
	unsigned char UnknownData00[0x78]; // 0x0028(0x0078) MISSED OFFSET
	TArray<class AActor*> AActors; // 0x00A0(0x0010)
	unsigned char UnknownData10[0x18]; // 0x00B0(0x0018) MISSED OFFSET
	struct ULevelActorContainer* ActorCluster; // 0xc8(0x08)
	struct UWorld* OwningWorld; // 0xd0(0x08)
	struct UModel* Model; // 0xd8(0x08)
	struct TArray<struct UModelComponent*> ModelComponents; // 0xe0(0x10)
	struct ALevelScriptActor* LevelScriptActor; // 0xf0(0x08)
	struct ANavigationObjectBase* NavListStart; // 0xf8(0x08)
	struct ANavigationObjectBase* NavListEnd; // 0x100(0x08)
	struct TArray<struct UNavigationDataChunk*> NavDataChunks; // 0x108(0x10)
	float LightmapTotalSize; // 0x118(0x04)
	float ShadowmapTotalSize; // 0x11c(0x04)
	struct TArray<struct FVector> StaticNavigableGeometry; // 0x120(0x10)
	char UnknownData_130[0x130]; // 0x130(0x130)
	bool LevelVisibility; // 0x260(0x01)
	char UnknownData_261[0xf]; // 0x261(0x0f)
	char UnknownData_270_0 : 3; // 0x270(0x01)
	char Blocked : 1; // 0x270(0x01)
	char UnknownData_270_4 : 4; // 0x270(0x01)
	char UnknownData_271[0x67]; // 0x271(0x67)
	struct TArray<struct UAssetUserData*> AssetUserData; // 0x2d8(0x10)
	char UnknownData_2E8[0x18]; // 0x2e8(0x18)
};*/

struct UPlayer {
	char UnknownData00[0x30];
	APlayerController* PlayerController;
};

// Class Engine.LocalPlayer
// Size: 0x210 (Inherited: 0x48)
struct ULocalPlayer : UPlayer {
	char UnknownData_48[0x18]; // 0x48(0x18)
	struct UGameViewportClient* ViewportClient; // 0x60(0x08)
	char UnknownData_68[0x40]; // 0x68(0x40)
	char AspectRatioAxisConstraint; // 0xa8(0x01)
	char UnknownData_A9[0x7]; // 0xa9(0x07)
	struct UClass* PendingLevelPlayerControllerClass; // 0xb0(0x08)
	char bSentSplitJoin : 1; // 0xb8(0x01)
	char UnknownData_B8_1 : 7; // 0xb8(0x01)
	char UnknownData_B9[0x3]; // 0xb9(0x03)
	float MinimumAspectRatio; // 0xbc(0x04)
	char UnknownData_C0[0x150]; // 0xc0(0x150)
};

struct UGameInstance {
	char UnknownData00[0x38];
	TArray<ULocalPlayer*> LocalPlayers; // 0x38
};

// Class Engine.World
// Size: 0x7c8 (Inherited: 0x28)
struct UWorld : UObject {
	static inline UWorld** GWorld = nullptr;
	char UnknownData_28[0x8]; // 0x28(0x08)
	struct ULevel* PersistentLevel; // 0x30(0x08)
	char pad3[0x20];
	AAthenaGameState* GameState; //0x0058
	char pad4[0xF0];
	TArray<ULevel*> Levels; //0x0150
	char pad5[0x50];
	ULevel* CurrentLevel; //0x01B0	
	char pad6[0x8];
	UGameInstance* GameInstance; //0x01C0
};


// Class CoreUObject.Interface
// Size: 0x28 (Inherited: 0x28)
struct UInterface : UObject {
};

// Class CoreUObject.GCObjectReferencer
// Size: 0x60 (Inherited: 0x28)
struct UGCObjectReferencer : UObject {
	char UnknownData_28[0x38]; // 0x28(0x38)
};

// Class CoreUObject.TextBuffer
// Size: 0x50 (Inherited: 0x28)
struct UTextBuffer : UObject {
	char UnknownData_28[0x28]; // 0x28(0x28)
};

// Class CoreUObject.Field
// Size: 0x30 (Inherited: 0x28)
struct UField : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Class CoreUObject.Struct
// Size: 0x88 (Inherited: 0x30)
struct UStruct : UField {
	UStruct* SuperField;
	char UnknownData_30[0x50]; // 0x28(0x50)
};

// Class CoreUObject.ScriptStruct
// Size: 0x98 (Inherited: 0x88)
struct UScriptStruct : UStruct {
	char UnknownData_88[0x10]; // 0x88(0x10)
};

// ScriptStruct AthenaEngine.SerialisedRpc
// Size: 0x20 (Inherited: 0x00)
struct FSerialisedRpc {
	char UnknownData_0[0x18]; // 0x00(0x18)
	class UScriptStruct* ContentsType; // 0x18(0x08)
};
// Class Engine.BlueprintFunctionLibrary
// Size: 0x28 (Inherited: 0x28)
struct UBlueprintFunctionLibrary : UObject {
};
// Class Athena.ObjectUtilities
// Size: 0x28 (Inherited: 0x28)
struct UObjectUtilities : UBlueprintFunctionLibrary {

	struct UObject* GetDefaultObject(struct UClass* Class); // Function Athena.ObjectUtilities.GetDefaultObject // Final|Native|Static|Public|BlueprintCallable|BlueprintPure // @ game+0x1064110
	struct UObject* CreateObject(struct UClass* Class)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ObjectUtilities.CreateObject");
		UClass* Character;
		Class = Character;
		ProcessEvent(this, fn, &Character);
	
	}; // Function Athena.ObjectUtilities.CreateObject // Final|Native|Static|Public|BlueprintCallable // @ game+0x1063610
};
// Class CoreUObject.Package
// Size: 0x80 (Inherited: 0x28)
struct UPackage : UObject {
	char UnknownData_28[0x58]; // 0x28(0x58)
};

// Class CoreUObject.Class
// Size: 0x1c0 (Inherited: 0x88)
class UClass : public UStruct
{
public:
	using UStruct::UStruct;
	unsigned char                                      UnknownData00[0x138];                                     // 0x0088(0x0138) MISSED OFFSET

	template<typename T>
	inline T* CreateDefaultObject()
	{
		return static_cast<T*>(CreateDefaultObject());
	}


	inline UObject* CreateDefaultObject()
	{
		return GetVFunction<UObject* (*)(UClass*)>(this, 88)(this);
	}

};

// Class Engine.ScriptViewportClient
// Size: 0x30 (Inherited: 0x28)
struct UScriptViewportClient : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Class Engine.GameViewportClient
// Size: 0x250 (Inherited: 0x30)
struct UGameViewportClient : UScriptViewportClient {
	char UnknownData_30[0x8]; // 0x30(0x08)
	struct UConsole* ViewportConsole; // 0x38(0x08)
	char DebugProperties[0x10]; // 0x40(0x10)
	char UnknownData_50[0x30]; // 0x50(0x30)
	struct UWorld* World; // 0x80(0x08)
	struct UGameInstance* GameInstance; // 0x88(0x08)
	char UnknownData_90[0x1c0]; // 0x90(0x1c0)

	void SSSwapControllers(); // Function Engine.GameViewportClient.SSSwapControllers // Exec|Native|Public // @ game+0x2fe5ca0
	void ShowTitleSafeArea(); // Function Engine.GameViewportClient.ShowTitleSafeArea // Exec|Native|Public // @ game+0x2fe5d50
	void SetConsoleTarget(int32_t PlayerIndex); // Function Engine.GameViewportClient.SetConsoleTarget // Exec|Native|Public // @ game+0x2fe5cc0
};

// Class Athena.AthenaGameViewportClient
// Size: 0x260 (Inherited: 0x250)
struct UAthenaGameViewportClient : UGameViewportClient {
	char UnknownData_250[0x10]; // 0x250(0x10)
};

// Class CoreUObject.Function
// Size: 0xb8 (Inherited: 0x88)
struct UFunction : UStruct {
	char UnknownData_88[0x30]; // 0x88(0x30)
};

// Class CoreUObject.DelegateFunction
// Size: 0xb8 (Inherited: 0xb8)
struct UDelegateFunction : UFunction {
};

// Class CoreUObject.SparseDelegateFunction
// Size: 0xc8 (Inherited: 0xb8)
struct USparseDelegateFunction : UDelegateFunction {
	char UnknownData_B8[0x10]; // 0xb8(0x10)
};

// Class CoreUObject.PackageMap
// Size: 0x50 (Inherited: 0x28)
struct UPackageMap : UObject {
	char UnknownData_28[0x28]; // 0x28(0x28)
};

// Class CoreUObject.Enum
// Size: 0x58 (Inherited: 0x30)
struct UEnum : UField {
	char UnknownData_30[0x28]; // 0x30(0x28)
};

// Class CoreUObject.LinkerPlaceholderClass
// Size: 0x310 (Inherited: 0x1c0)
struct ULinkerPlaceholderClass : UClass {
	char UnknownData_1C0[0x150]; // 0x1c0(0x150)
};

// Class CoreUObject.LinkerPlaceholderExportObject
// Size: 0x88 (Inherited: 0x28)
struct ULinkerPlaceholderExportObject : UObject {
	char UnknownData_28[0x60]; // 0x28(0x60)
};

// Class CoreUObject.LinkerPlaceholderFunction
// Size: 0x208 (Inherited: 0xb8)
struct ULinkerPlaceholderFunction : UFunction {
	char UnknownData_B8[0x150]; // 0xb8(0x150)
};

// Class CoreUObject.MetaData
// Size: 0x78 (Inherited: 0x28)
struct UMetaData : UObject {
	char UnknownData_28[0x50]; // 0x28(0x50)
};

// Class CoreUObject.ObjectRedirector
// Size: 0x30 (Inherited: 0x28)
struct UObjectRedirector : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Class CoreUObject.Property
// Size: 0x70 (Inherited: 0x30)
struct UProperty : UField {
	char UnknownData_30[0x40]; // 0x30(0x40)
};

// Class CoreUObject.NumericProperty
// Size: 0x70 (Inherited: 0x70)
struct UNumericProperty : UProperty {
};

// Class CoreUObject.ArrayProperty
// Size: 0x78 (Inherited: 0x70)
struct UArrayProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.ObjectPropertyBase
// Size: 0x78 (Inherited: 0x70)
struct UObjectPropertyBase : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.AssetObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct UAssetObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.AssetClassProperty
// Size: 0x80 (Inherited: 0x78)
struct UAssetClassProperty : UAssetObjectProperty {
	char UnknownData_78[0x8]; // 0x78(0x08)
};

// Class CoreUObject.BoolProperty
// Size: 0x78 (Inherited: 0x70)
struct UBoolProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.ByteProperty
// Size: 0x78 (Inherited: 0x70)
struct UByteProperty : UNumericProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.ObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct UObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.ClassProperty
// Size: 0x80 (Inherited: 0x78)
struct UClassProperty : UObjectProperty {
	char UnknownData_78[0x8]; // 0x78(0x08)
};

// Class CoreUObject.DelegateProperty
// Size: 0x78 (Inherited: 0x70)
struct UDelegateProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.DoubleProperty
// Size: 0x70 (Inherited: 0x70)
struct UDoubleProperty : UNumericProperty {
};

// Class CoreUObject.FloatProperty
// Size: 0x70 (Inherited: 0x70)
struct UFloatProperty : UNumericProperty {
};

// Class CoreUObject.IntProperty
// Size: 0x70 (Inherited: 0x70)
struct UIntProperty : UNumericProperty {
};

// Class CoreUObject.Int16Property
// Size: 0x70 (Inherited: 0x70)
struct UInt16Property : UNumericProperty {
};

// Class CoreUObject.Int64Property
// Size: 0x70 (Inherited: 0x70)
struct UInt64Property : UNumericProperty {
};

// Class CoreUObject.Int8Property
// Size: 0x70 (Inherited: 0x70)
struct UInt8Property : UNumericProperty {
};

// Class CoreUObject.InterfaceProperty
// Size: 0x78 (Inherited: 0x70)
struct UInterfaceProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.LazyObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct ULazyObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.MapProperty
// Size: 0xa8 (Inherited: 0x70)
struct UMapProperty : UProperty {
	char UnknownData_70[0x38]; // 0x70(0x38)
};

// Class CoreUObject.MulticastDelegateProperty
// Size: 0x78 (Inherited: 0x70)
struct UMulticastDelegateProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.MulticastInlineDelegateProperty
// Size: 0x78 (Inherited: 0x78)
struct UMulticastInlineDelegateProperty : UMulticastDelegateProperty {
};

// Class CoreUObject.NameProperty
// Size: 0x70 (Inherited: 0x70)
struct UNameProperty : UProperty {
};

// Class CoreUObject.StrProperty
// Size: 0x70 (Inherited: 0x70)
struct UStrProperty : UProperty {
};

// Class CoreUObject.StructProperty
// Size: 0x78 (Inherited: 0x70)
struct UStructProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.UInt16Property
// Size: 0x70 (Inherited: 0x70)
struct UUInt16Property : UNumericProperty {
};

// Class CoreUObject.UInt32Property
// Size: 0x70 (Inherited: 0x70)
struct UUInt32Property : UNumericProperty {
};

// Class CoreUObject.UInt64Property
// Size: 0x70 (Inherited: 0x70)
struct UUInt64Property : UNumericProperty {
};

// Class CoreUObject.WeakObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct UWeakObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.TextProperty
// Size: 0x70 (Inherited: 0x70)
struct UTextProperty : UProperty {
};

struct UCrewFunctions {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Athena.CrewFunctions");
	}
	static bool AreCharactersInSameCrew(ACharacter* Player1, ACharacter* Player2) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.CrewFunctions.AreCharactersInSameCrew");
		struct
		{
			ACharacter* Player1;
			ACharacter* Player2;
			bool ReturnValue;
		} params;
		params.Player1 = Player1;
		params.Player2 = Player2;
		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}
};

struct FAggressiveGhostShipState {
	bool IsShipVisible; // 0x00(0x01)
	bool IsShipDead; // 0x01(0x01)
	char UnknownData_2[0x2]; // 0x02(0x02)
	float ShipSpeed; // 0x04(0x04)
};

// Class Athena.AggressiveGhostShip
// Size: 0x880 (Inherited: 0x3d0)
struct AAggressiveGhostShip : AActor {
	char UnknownData_3D0[0x150]; // 0x3d0(0x40)
	struct FAggressiveGhostShipState ShipState; // 0x520(0x08)
	char UnknownData_530[0x14]; // 0x528(0x14)
	int32_t NumShotsLeftToKill; // 0x53C(0x04)
	char UnknownData_548[0x340]; // 0x540(0x338)
};

enum class EDrawDebugTrace : uint8_t
{
	EDrawDebugTrace__None = 0,
	EDrawDebugTrace__ForOneFrame = 1,
	EDrawDebugTrace__ForDuration = 2,
	EDrawDebugTrace__Persistent = 3,
	EDrawDebugTrace__EDrawDebugTrace_MAX = 4
};


enum class ETraceTypeQuery : uint8_t
{
	TraceTypeQuery1 = 0,
	TraceTypeQuery2 = 1,
	TraceTypeQuery3 = 2,
	TraceTypeQuery4 = 3,
	TraceTypeQuery5 = 4,
	TraceTypeQuery6 = 5,
	TraceTypeQuery7 = 6,
	TraceTypeQuery8 = 7,
	TraceTypeQuery9 = 8,
	TraceTypeQuery10 = 9,
	TraceTypeQuery11 = 10,
	TraceTypeQuery12 = 11,
	TraceTypeQuery13 = 12,
	TraceTypeQuery14 = 13,
	TraceTypeQuery15 = 14,
	TraceTypeQuery16 = 15,
	TraceTypeQuery17 = 16,
	TraceTypeQuery18 = 17,
	TraceTypeQuery19 = 18,
	TraceTypeQuery20 = 19,
	TraceTypeQuery21 = 20,
	TraceTypeQuery22 = 21,
	TraceTypeQuery23 = 22,
	TraceTypeQuery24 = 23,
	TraceTypeQuery25 = 24,
	TraceTypeQuery26 = 25,
	TraceTypeQuery27 = 26,
	TraceTypeQuery28 = 27,
	TraceTypeQuery29 = 28,
	TraceTypeQuery30 = 29,
	TraceTypeQuery31 = 30,
	TraceTypeQuery32 = 31,
	TraceTypeQuery_MAX = 32,
	ETraceTypeQuery_MAX = 33
};


class UKismetSystemLibrary {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetSystemLibrary");
	}

	static FRotator DrawDebugCircle(struct UObject* WorldContextObject, struct FVector Center, float Radius, int32_t NumSegments, struct FLinearColor LineColor, float Duration, float Thickness, struct FVector YAxis, struct FVector ZAxis, bool bDrawAxis) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugCircle");

		struct {
			struct UObject* WorldContextObject;
			struct FVector Center;
			float Radius;
			int32_t NumSegments;
			struct FLinearColor LineColor;
			float Duration;
			float Thickness;
			struct FVector YAxis;
			struct FVector ZAxis;
			bool bDrawAxis;
		} params;
		params.WorldContextObject = WorldContextObject;
		params.Center = Center;
		params.Radius = Radius;
		params.NumSegments = NumSegments;
		params.LineColor = LineColor;
		params.Duration = Duration;
		params.Thickness = Thickness;
		params.YAxis = YAxis;
		params.ZAxis = ZAxis;
		params.bDrawAxis = bDrawAxis;

		ProcessEvent(defaultObj, fn, &params);
	}
	static FRotator DrawDebugBox(struct UObject* WorldContextObject, struct FVector Center, struct FVector Extent, struct FLinearColor LineColor, struct FRotator Rotation, float Duration)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugBox");
		struct {
			struct UObject* WorldContextObject;
			struct FVector Center;
			struct FVector Extent;
			struct FLinearColor LineColor;
			struct FRotator Rotation;
			float Duration;
		} params;
		params.WorldContextObject = WorldContextObject;
		params.Center = Center;
		params.Extent = Extent;
		params.LineColor = LineColor;
		params.Rotation = Rotation;
		params.Duration = Duration;
		ProcessEvent(defaultObj, fn, &params);
	}; 
};

class UKismetMathLibrary {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetMathLibrary");
	}
	static FRotator NormalizedDeltaRotator(const struct FRotator& A, const struct FRotator& B) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.NormalizedDeltaRotator");

		struct
		{
			struct FRotator                A;
			struct FRotator                B;
			struct FRotator                ReturnValue;
		} params;

		params.A = A;
		params.B = B;

		ProcessEvent(defaultObj, fn, &params);

		return params.ReturnValue;

	};
	static FRotator FindLookAtRotation(const FVector& Start, const FVector& Target) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.FindLookAtRotation");

		struct {
			FVector Start;
			FVector Target;
			FRotator ReturnValue;
		} params;
		params.Start = Start;
		params.Target = Target;

		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}

	static FVector Conv_RotatorToVector(const struct FRotator& InRot) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.Conv_RotatorToVector");

		struct
		{
			struct FRotator                InRot;
			struct FVector                 ReturnValue;
		} params;
		params.InRot = InRot;

		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}

	static bool LineTraceSingle_NEW(class UObject* WorldContextObject, const struct FVector& Start, const struct FVector& End, ETraceTypeQuery TraceChannel, bool bTraceComplex, TArray<class AActor*> ActorsToIgnore, EDrawDebugTrace DrawDebugType, bool bIgnoreSelf, struct FHitResult* OutHit)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.LineTraceSingle_NEW");

		struct
		{
			class UObject* WorldContextObject;
			struct FVector                 Start;
			struct FVector                 End;
			ETraceTypeQuery				   TraceChannel;
			bool                           bTraceComplex;
			TArray<class AActor*>          ActorsToIgnore;
			EDrawDebugTrace				   DrawDebugType;
			struct FHitResult              OutHit;
			bool                           bIgnoreSelf;
			bool                           ReturnValue;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.Start = Start;
		params.End = End;
		params.TraceChannel = TraceChannel;
		params.bTraceComplex = bTraceComplex;
		params.ActorsToIgnore = ActorsToIgnore;
		params.DrawDebugType = DrawDebugType;
		params.bIgnoreSelf = bIgnoreSelf;

		ProcessEvent(defaultObj, fn, &params);

		if (OutHit != nullptr)
			*OutHit = params.OutHit;

		return params.ReturnValue;
	}

	static void DrawDebugBox(UObject* WorldContextObject, const FVector& Center, const FVector& Extent, const FLinearColor& LineColor, const FRotator& Rotation, float Duration) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugBox");
		struct
		{
			UObject* WorldContextObject = nullptr;
			FVector Center;
			FVector Extent;
			FLinearColor LineColor;
			FRotator Rotation;
			float Duration = INFINITY;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.Center = Center;
		params.Extent = Extent;
		params.LineColor = LineColor;
		params.Rotation = Rotation;
		params.Duration = Duration;
		ProcessEvent(defaultObj, fn, &params);
	}
	static void DrawDebugArrow(UObject* WorldContextObject, const FVector& LineStart, const FVector& LineEnd, float ArrowSize, const FLinearColor& LineColor, float Duration) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugBox");
		struct
		{
			class UObject* WorldContextObject = nullptr;
			struct FVector LineStart;
			struct FVector LineEnd;
			float ArrowSize = 1.f;
			struct FLinearColor LineColor;
			float Duration = 1.f;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.LineStart = LineStart;
		params.LineEnd = LineEnd;
		params.ArrowSize = ArrowSize;
		params.LineColor = LineColor;
		params.Duration = Duration;

		ProcessEvent(defaultObj, fn, &params);
	}
};

enum class EMeleeWeaponMovementSpeed : uint8_t
{
	EMeleeWeaponMovementSpeed__Default = 0,
	EMeleeWeaponMovementSpeed__SlightlySlowed = 1,
	EMeleeWeaponMovementSpeed__Slowed = 2,
	EMeleeWeaponMovementSpeed__EMeleeWeaponMovementSpeed_MAX = 3
};

struct UMeleeAttackDataAsset
{
	char pad0[0x238];
	float                                              ClampYawRange;                                             // 0x0238(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              ClampYawRate;                                              // 0x023C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};
/*
char pad1[0x7];
float ComboTime; // 0x54(0x04)
char pad2[0x1E0];
char pad1[0x01E8];
*/
struct UMeleeWeaponDataAsset
{
	char pad[0x0048];
	class UMeleeAttackDataAsset* HeavyAttack; //0x0048
	char pad2[0x0028];
	EMeleeWeaponMovementSpeed BlockingMovementSpeed; //0x0078
};

struct AMeleeWeapon
{
	char pad[0x7c8];
	struct UMeleeWeaponDataAsset* DataAsset; // 0x7c8(0x08)
};

// ScriptStruct Athena.ProjectileShotParams
// Size: 0x1c (Inherited: 0x00)
struct FProjectileShotParams {
	int Seed; // 0x00(0x04)
	float ProjectileDistributionMaxAngle; // 0x04(0x04)
	int NumberOfProjectiles; // 0x08(0x04)
	float ProjectileMaximumRange; // 0x0c(0x04)
	float ProjectileHitScanMaximumRange; // 0x10(0x04)
	float ProjectileDamage; // 0x14(0x04)
	float ProjectileDamageMultiplierAtMaximumRange; // 0x18(0x04)
};
// ScriptStruct Athena.MeleeAttackHitResponses
// Size: 0x01 (Inherited: 0x00)
struct FMeleeAttackHitResponses {
	char DecalsParticlesAudio : 1; // 0x00(0x01)
	char Animation : 1; // 0x00(0x01)
	char CameraShake : 1; // 0x00(0x01)
	char GamepadRumble : 1; // 0x00(0x01)
	char HitIndicator : 1; // 0x00(0x01)
	char UnknownData_0_5 : 3; // 0x00(0x01)
};

// ScriptStruct Athena.WeaponProjectileParams
// Size: 0xc0 (Inherited: 0x00)
struct FWeaponProjectileParams {
	float Damage; // 0x00(0x04)
	float DamageMultiplierAtMaximumRange; // 0x04(0x04)
	float LifeTime; // 0x08(0x04)
	float TrailFadeOutTime; // 0x0c(0x04)
	float Velocity; // 0x10(0x04)
	float TimeBeforeApplyingGravity; // 0x14(0x04)
	float DownForceVelocityFractionPerSecond; // 0x18(0x04)
	float VelocityDampeningPerSecond; // 0x1c(0x04)
	struct FLinearColor Color; // 0x20(0x10)
	struct UClass* ProjectileId; // 0x30(0x08)
	char HealthChangeReason; // 0x38(0x01)UWorld
	char UnknownData_39[0x3]; // 0x39(0x03)
	char UnknownData_3C[0x68]; // 0x3C(0x68)
	int SuggestedMaxSimulationIterations; // 0xa4(0x04)
	float SuggestedMinTickTimeSecs; // 0xa8(0x04)
	float SuggestedMaxSimulationTimeStep; // 0xac(0x04)
	float HitScanTrailUpdateModifier; // 0xb0(0x04)
	float HitScanTrailFadeOutTime; // 0xb4(0x04)
	float HitScanTrailGrowthSpeed; // 0xb8(0x04)
	char UnknownData_BC[0x4]; // 0xbc(0x04)
};
// ScriptStruct Athena.ProjectileWeaponParameters
// Size: 0x1e8 (Inherited: 0x00)
struct FProjectileWeaponParameters {
	int32_t AmmoClipSize; // 0x00(0x04)
	int32_t AmmoCostPerShot; // 0x04(0x04)
	float EquipDuration; // 0x08(0x04)
	float IntoAimingDuration; // 0x0c(0x04)
	float RecoilDuration; // 0x10(0x04)
	float ReloadDuration; // 0x14(0x04)
	char HipFireProjectileShotParams[0x1c]; // 0x18(0x1c)
	char AimDownSightsProjectileShotParams[0x1c]; // 0x34(0x1c)
	int32_t InaccuracySeed; // 0x50(0x04)
	float ProjectileDistributionMaxAngle; // 0x54(0x04)
	int32_t NumberOfProjectiles; // 0x58(0x04)
	float ProjectileMaximumRange; // 0x5c(0x04)
	float ProjectileHitScanMaximumRange; // 0x60(0x04)
	float ProjectileDamage; // 0x64(0x04)
	float ProjectileDamageMultiplierAtMaximumRange; // 0x68(0x04)
	char UnknownData_6C[0x4]; // 0x6c(0x04)
	struct UClass* DamagerType; // 0x70(0x08)
	struct UClass* ProjectileId; // 0x78(0x08)
	struct FWeaponProjectileParams AmmoParams; // 0x80(0xb0)
	bool UsesScope; // 0x130(0x01)
	char UnknownData_131[0x3]; // 0x131(0x03)
	float ZoomedRecoilDurationIncrease; // 0x134(0x04)
	float SecondsUntilZoomStarts; // 0x138(0x04)
	float SecondsUntilPostStarts; // 0x13c(0x04)
	float WeaponFiredAINoiseRange; // 0x140(0x04)
	float MaximumRequestPositionDelta; // 0x144(0x04)
	float MaximumRequestAngleDelta; // 0x148(0x04)
	float TimeoutTolerance; // 0x14c(0x04)
	float AimingMoveSpeedScalar; // 0x150(0x04)
	char AimSensitivitySettingCategory; // 0x154(0x01)
	char UnknownData_155[0x3]; // 0x155(0x03)
	float InAimFOV; // 0x158(0x04)
	float BlendSpeed; // 0x15c(0x04)
	char DryFireSfx[0x8]; // 0x160(0x08)
	char AudioEmitterParameters[0x10]; // 0x168(0x10)
	struct FName RumbleTag; // 0x178(0x08)
	bool KnockbackEnabled; // 0x180(0x01)
	char UnknownData_181[0x3]; // 0x181(0x03)
	char KnockbackParams[0x50]; // 0x184(0x50)
	bool StunEnabled; // 0x1d4(0x01)
	char UnknownData_1D5[0x3]; // 0x1d5(0x03)
	float StunDuration; // 0x1d8(0x04)
	struct FVector TargetingOffset; // 0x1dc(0x0c)
};

struct AProjectileWeapon {
	char pad[0x7e8]; // 0
	FProjectileWeaponParameters WeaponParameters; // 0x7e8
	char State; // 0x9b0(0x01)
	char UnknownData_9D1[0x3]; // 0x9d1(0x03)
	int32_t AmmoLeft; // 0x9b4(0x04)
	char AimSensitivityComponent[0x8]; // 0x9d8(0x08)
	char UnknownData_9E0[0x10]; // 0x9e0(0x10)
	char ProjectileWeaponType; // 0x9d0(0x01)

	bool CanFire()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ProjectileWeapon.CanFire");
		bool canfire;
		ProcessEvent(this, fn, &canfire);
		return canfire;
	}


};

// Class Athena.TestProjectileWeapon
// Size: 0xb30 (Inherited: 0xa50)
struct ATestProjectileWeapon : AProjectileWeapon {
	void FireInstantly()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.TestProjectileWeapon.FireInstantly");
		ProcessEvent(this, fn, nullptr);
	}
};



struct AMapTable
{
	char pad[0x04E8];
	struct TArray<struct FVector2D> MapPins; // 0x4e8(0x10)
	struct TArray<struct FWorldMapShipLocation> TrackedShips; // 0x4f8(0x10)
};

struct AMapTableESP
{
	char pad[0x0498];
	bool DisplayFactionShips; // 0x498(0x01)
};


// Enum Cooking.ECookingSmokeFeedbackLevel
enum class ECookingSmokeFeedbackLevel : uint8_t
{
	ECookingSmokeFeedbackLevel__NotCooking = 0,
	ECookingSmokeFeedbackLevel__Raw = 1,
	ECookingSmokeFeedbackLevel__CookedWarning = 2,
	ECookingSmokeFeedbackLevel__Cooked = 3,
	ECookingSmokeFeedbackLevel__BurnedWarning = 4,
	ECookingSmokeFeedbackLevel__Burned = 5,
	ECookingSmokeFeedbackLevel__ECookingSmokeFeedbackLevel_MAX = 6
};

template<class TEnum>
class TEnumAsByte
{
public:
	inline TEnumAsByte()
	{
	}

	inline TEnumAsByte(TEnum _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(int32_t _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(uint8_t _value)
		: value(_value)
	{
	}

	inline operator TEnum() const
	{
		return (TEnum)value;
	}

	inline TEnum GetValue() const
	{
		return (TEnum)value;
	}

private:
	uint8_t value;
};

// ScriptStruct Cooking.CookingClientRepresentation
// 0x00A8
struct FCookingClientRepresentation
{
	bool                                               Cooking;                                                  // 0x0000(0x0001) (ZeroConstructor, IsPlainOldData)
	bool                                               HasItem;                                                  // 0x0001(0x0001) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData00[0x6];                                       // 0x0002(0x0006) MISSED OFFSET
	struct FText                                       CurrentCookingItemDisplayName;                            // 0x0008(0x0038)
	class UClass* CurrentCookingItemCategory;                               // 0x0040(0x0008) (ZeroConstructor, IsPlainOldData)
	TEnumAsByte<ECookingSmokeFeedbackLevel>  		SmokeFeedbackLevel;                                       // 0x0048(0x0001) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData01[0x3];                                       // 0x0049(0x0003) MISSED OFFSET
	float                                              VisibleCookedExtent;                                      // 0x004C(0x0004) (ZeroConstructor, IsPlainOldData)
	char               VisibleMaterialSettings[0x30];                                  // 0x0050(0x0030)
	class UMaterialInstance* OverrideMaterial;                                         // 0x0080(0x0008) (ZeroConstructor, IsPlainOldData)
	struct FVector                                     BurnDownVector;                                           // 0x0088(0x000C) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData02[0x4];                                       // 0x0094(0x0004) MISSED OFFSET
	class UAnimationAsset* AnimatedPose;                                             // 0x0098(0x0008) (ZeroConstructor, IsPlainOldData)
	struct FName                                       CurrentCookableTypeName;                                  // 0x00A0(0x0008) (ZeroConstructor, IsPlainOldData)
};

// Class Cooking.CookerComponent
// Size: 0x248 (Inherited: 0xc8)
struct UCookerComponent {
	char UnknownData_C8[0xd0]; // 0x0(0xd0)
	struct TArray<struct FStatus> StatusToApplyToContents; // 0xd0(0x10)
	struct TArray<struct FCookerSmokeFeedbackEntry> VFXFeedback; // 0xe0(0x10)
	struct UStaticMeshMemoryConstraintComponent* CookableStaticMeshComponent; // 0xf0(0x08)
	struct USkeletalMeshMemoryConstraintComponent* CookableSkeletalMeshComponent; // 0xf8(0x08)
	struct FName CookedMaterialParameterName; // 0x100(0x08)
	struct FName BurnDownDirectionParameterName; // 0x108(0x08)
	float PlacementVarianceAngleBound; // 0x110(0x04)
	bool OnByDefault; // 0x114(0x01)
	char UnknownData_115[0x3]; // 0x115(0x03)
	struct UCookingComponentAudioParams* AudioParams; // 0x118(0x08)
	char VfxLocation; // 0x120(0x01)
	char UnknownData_121[0x7]; // 0x121(0x07)
	struct AItemInfo* CurrentlyCookingItem; // 0x128(0x08)
	struct FCookingClientRepresentation CookingState; // 0x130(0x68)
	struct UParticleSystemComponent* SmokeParticleComponent; // 0x198(0x08)
	struct UMaterialInstanceDynamic* VisibleCookableMaterial; // 0x1a0(0x08)
	bool TurnedOn; // 0x1a8(0x01)
	bool OnIsland; // 0x1a9(0x01)
	char UnknownData_1AA[0x9e]; // 0x1aa(0x9e)

	void OnRep_CookingState(struct FCookingClientRepresentation OldRepresentation); // Function Cooking.CookerComponent.OnRep_CookingState // Final|Native|Private|HasOutParms // @ game+0x360d390
};

// Class Cooking.CookingPot
// Size: 0x5d8 (Inherited: 0x3f8)
struct ACookingPot {
	char UnknownData_3F8[0x400]; // 0x0(0x400)
	struct UStaticMeshComponent* MeshComponent; // 0x400(0x08)
	struct UActionRulesInteractableComponent* InteractableComponent; // 0x408(0x08)
	struct UCookerComponent* CookerComponent; // 0x410(0x08)
	float HoldToInteractTime; // 0x418(0x04)
	char UnknownData_41C[0x4]; // 0x41c(0x04)
	struct FText NotWieldingCookableItemTooltip; // 0x420(0x38)
	struct FText WieldingCookableItemTooltip; // 0x458(0x38)
	struct FText TakeItemTooltip; // 0x490(0x38)
	struct FText CannotTakeItemTooltip; // 0x4c8(0x38)
	struct FText MixInItemTooltip; // 0x500(0x38)
	char UnknownData_538[0xa0]; // 0x538(0xa0)
};


// Class Cooking.CookableComponent
// Size: 0x128 (Inherited: 0xc8)
struct UCookableComponent {
	char UnknownData_C8[0xe8]; // 0xc8(0x20)
	struct UClass* NextCookState; // 0xe8(0x08)
	float TimeToNextCookState; // 0xf0(0x04)
	char UnknownData_F4[0x4]; // 0xf4(0x04)
	struct TArray<struct FCookableComponentSmokeFeedbackTimingEntry> SmokeFeedbackLevels; // 0xf8(0x10)
	struct UCurveFloat* VisibleCookedExtentOverTime; // 0x108(0x08)
	float DefaultVisibleCookedExtent; // 0x110(0x04)
	struct FName CookableTypeName; // 0x114(0x08)
	//struct FPlayerStat CookedStat; // 0x11c(0x04)
	//struct FPlayerStat ShipCookedStat; // 0x120(0x04)
	char CookedStat[0x04]; // 0x11c(0x04)
	char ShipCookedStat[0x04]; // 0x120(0x04)
	char CookingState; // 0x124(0x01)
	char InitialCookingState; // 0x125(0x01)
	char RemovedCookingState; // 0x126(0x01)
	char UnknownData_127[0x1]; // 0x127(0x01)
};

// Class Engine.CharacterMovementComponent
// Size: 0x590 (Inherited: 0x160)
struct UCharacterMovementComponent {
	char pad_00[0x160];
	char UnknownData_160[0x18]; // 0x160(0x18)
	struct ACharacter* CharacterOwner; // 0x178(0x08)
	float GravityScale; // 0x180(0x04)
	float MaxStepHeight; // 0x184(0x04)
	float JumpZVelocity; // 0x188(0x04)
	float JumpOffJumpZFactor; // 0x18c(0x04)
	float WalkableFloorAngle; // 0x190(0x04)
	float WalkableFloorZ; // 0x194(0x04)
	char MovementMode; // 0x198(0x01)
	char CustomMovementMode; // 0x199(0x01)
	char UnknownData_19A[0x26]; // 0x19a(0x26)
	float GroundFriction; // 0x1c0(0x04)
	float MaxWalkSpeed; // 0x1c4(0x04)
	float MaxWalkSpeedBackwards; // 0x1c8(0x04)
	float WalkBackwardsMinAngle; // 0x1cc(0x04)
	float WalkBackwardsMaxAngle; // 0x1d0(0x04)
	float MaxWalkSpeedCrouched; // 0x1d4(0x04)
	float MaxSwimSpeed; // 0x1d8(0x04)
	float MaxFlySpeed; // 0x1dc(0x04)
};

// Class Athena.OnlineAthenaPlayerController
// Size: 0x1618 (Inherited: 0x1550)
struct AOnlineAthenaPlayerController {
	char pad_00[0x1550];
	char UnknownData_1550[0x8]; // 0x1550(0x08)
	char LogoutNoteCompletionIdent[0x8]; // 0x1558(0x08)
	char UnknownData_1560[0x19]; // 0x1560(0x19)
	bool IdleDisconnectEnabled; // 0x1579(0x01)
	char UnknownData_157A[0x9e]; // 0x157a(0x9e)
};

// Class Athena.Spyglass
// Size: 0x870 (Inherited: 0x780)
struct ASpyglass {
	char pad_00[0x780];
	char UnknownData_780[0x10]; // 0x780(0x10)
	char InventoryItem[0x8]; // 0x790(0x08)
	char UsableWieldableComponent[0x8]; // 0x798(0x08)
	char AimSensitivityComponent[0x8]; // 0x7a0(0x08)
	char UnknownData_7A8[0x10]; // 0x7a8(0x10)
	float NameplateVisibilityRangeExtensionFactorWhileZoomed; // 0x7b8(0x04)
	float SecondsUntilZoomStarts; // 0x7bc(0x04)
	float SecondsUntilPostStarts; // 0x7c0(0x04)
	float InAimFOV; // 0x7c4(0x04)
	float BlendSpeed; // 0x7c8(0x04)
	char AimSpeedScaleParameters[0x8]; // 0x7cc(0x08)
	char UnknownData_7D4[0x4]; // 0x7d4(0x04)
	char PostProcessComponent[0x8]; // 0x7d8(0x08)
	char BlurCurve[0x8]; // 0x7e0(0x08)
	char UnknownData_7E8[0x38]; // 0x7e8(0x38)
	char DynamicMaterial[0x8]; // 0x820(0x08)
	bool TurningOn; // 0x828(0x01)
	char UnknownData_829[0x3]; // 0x829(0x03)
	float BlurTime; // 0x82c(0x04)
	float BlurInDuration; // 0x830(0x04)
	float BlurOutDuration; // 0x834(0x04)
	char Glint[0x8]; // 0x838(0x08)
	char MaterialParent[0x8]; // 0x840(0x08)
	char LensNormal[0x8]; // 0x848(0x08)
	char LensMask[0x8]; // 0x850(0x08)
	char LensTint[0x10]; // 0x858(0x10)
	char UnknownData_868[0x8]; // 0x868(0x08)

	void SpyGlassRaisedFirstPerson(bool IsRaised); // Function Athena.Spyglass.SpyGlassRaisedFirstPerson // Event|Protected|BlueprintEvent // @ game+0x179fa40
	void SetGlintOff(); // Function Athena.Spyglass.SetGlintOff // Native|Public|BlueprintCallable // @ game+0x11489f0
	void ResetSpyglassEffects(); // Function Athena.Spyglass.ResetSpyglassEffects // Final|Native|Public|BlueprintCallable // @ game+0x1148200
};

struct FSailsBillowStateChangeRpc {
	char pad_00[0x10];
	bool AreSailsBillowed; // 0x10(0x01)
};

class UFunctionEx : public UStruct
{
public:
	int FunctionFlags;
	uint16_t RepOffset;
	uint8_t NumParms;
	char pad;
	uint16_t ParmsSize;
	uint16_t ReturnValueOffset;
	uint16_t RPCId;
	uint16_t RPCResponseId;
	UProperty* FirstPropertyToInit;
	UFunctionEx* EventGraphFunction; //0x00A0
	int EventGraphCallOffset;
	char pad_0x00AC[0x4]; //0x00AC
	void* Func; //0x00B0
};

// ScriptStruct Athena.LightingControllerAssetParams
// Size: 0x50 (Inherited: 0x00)
struct FLightingControllerAssetParams {
	struct UMaterialParameterCollection* AtmosphericMaterialParams; // 0x00(0x08)
	struct UMaterialParameterCollection* CloudMaterialParams; // 0x08(0x08)
	struct UMaterialParameterCollection* ScreenEffectsMaterialParams; // 0x10(0x08)
	struct UMaterialParameterCollection* IslandDangerMaterialParams; // 0x18(0x08)
	struct ULightingControllerSettings* UnderworldLightingSet; // 0x20(0x08)
	struct ULightingControllerSettings* HideoutLightingSet; // 0x28(0x08)
	struct ULightingControllerSettings* ArenaTavernLightingSet; // 0x30(0x08)
	struct UStaticMesh* SkyDomeMesh; // 0x38(0x08)
	struct UStaticMesh* NorthStarMesh; // 0x40(0x08)
	struct UStaticMesh* MoonMesh; // 0x48(0x08)
};
struct ALightingController {
	char pad_0[0x728];
	float FixedTimeOfDay; // 0x728(0x04)
	int32_t FixedDay; // 0x72c(0x04)
	bool IsFixedTimeOfDay; // 0x730(0x01)
	char UnknownData_731[0x7]; // 0x731(0x07)
	struct USceneComponent* RootSceneComponent; // 0x738(0x08)
	struct FLightingControllerAssetParams AssetParams; // 0x740(0x50)
	struct UDirectionalLightComponent* DirectionalLight; // 0x790(0x08)
	struct UStaticMeshComponent* SkyDome; // 0x798(0x08)
	struct UStaticMeshComponent* NorthStar; // 0x7a0(0x08)
	struct UExponentialHeightFogComponent* Fog; // 0x7a8(0x08)
	struct UExponentialHeightFogComponent* UnderwaterFog; // 0x7b0(0x08)
	struct UPostProcessComponent* GlobalPostProcess; // 0x7b8(0x08)
	struct UDirectionalLightComponent* RainLight; // 0x7c0(0x08)
	struct UStaticMeshComponent* Moon; // 0x7c8(0x08)
	struct USkyLightComponent* SkyLight; // 0x7d0(0x08)

};
// ScriptStruct Engine.LightmassMaterialInterfaceSettings
// Size: 0x14 (Inherited: 0x00)
struct FLightmassMaterialInterfaceSettings {
	char bCastShadowAsMasked : 1; // 0x00(0x01)
	char UnknownData_0_1 : 7; // 0x00(0x01)
	char UnknownData_1[0x3]; // 0x01(0x03)
	float EmissiveBoost; // 0x04(0x04)
	float DiffuseBoost; // 0x08(0x04)
	float ExportResolutionScale; // 0x0c(0x04)
	char bOverrideCastShadowAsMasked : 1; // 0x10(0x01)
	char bOverrideEmissiveBoost : 1; // 0x10(0x01)
	char bOverrideDiffuseBoost : 1; // 0x10(0x01)
	char bOverrideExportResolutionScale : 1; // 0x10(0x01)
	char UnknownData_10_4 : 4; // 0x10(0x01)
	char UnknownData_11[0x3]; // 0x11(0x03)
};
// Class Engine.MaterialInterface
// Size: 0x58 (Inherited: 0x28)
struct UMaterialInterface : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
	struct USubsurfaceProfile* SubsurfaceProfile; // 0x30(0x08)
	char UnknownData_38[0x4]; // 0x38(0x04)
	struct FLightmassMaterialInterfaceSettings LightmassSettings; // 0x3c(0x14)
	char UnknownData_50[0x8]; // 0x50(0x08)

	struct UPhysicalMaterial* GetPhysicalMaterial(); // Function Engine.MaterialInterface.GetPhysicalMaterial // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0b020
	struct UMaterial* GetBaseMaterial(); // Function Engine.MaterialInterface.GetBaseMaterial // Final|RequiredAPI|Native|Public|BlueprintCallable // @ game+0x2d08b80
};
// ScriptStruct CoreUObject.Color
// Size: 0x04 (Inherited: 0x00)
struct FColor {
	char B; // 0x00(0x01)
	char G; // 0x01(0x01)
	char R; // 0x02(0x01)
	char A; // 0x03(0x01)
};
// ScriptStruct Engine.MaterialInput
// Size: 0x38 (Inherited: 0x00)
struct FMaterialInput {
	struct UMaterialExpression* Expression; // 0x00(0x08)
	int32_t OutputIndex; // 0x08(0x04)
	char UnknownData_C[0x4]; // 0x0c(0x04)
	struct FString InputName; // 0x10(0x10)
	int32_t Mask; // 0x20(0x04)
	int32_t MaskR; // 0x24(0x04)
	int32_t MaskG; // 0x28(0x04)
	int32_t MaskB; // 0x2c(0x04)
	int32_t MaskA; // 0x30(0x04)
	int32_t GCC64_Padding; // 0x34(0x04)
};
// ScriptStruct Engine.ColorMaterialInput
// Size: 0x40 (Inherited: 0x38)
struct FColorMaterialInput : FMaterialInput {
	char UseConstant : 1; // 0x38(0x01)
	char UnknownData_38_1 : 7; // 0x38(0x01)
	char UnknownData_39[0x3]; // 0x39(0x03)
	struct FColor Constant; // 0x3c(0x04)
};
// ScriptStruct Engine.ScalarMaterialInput
// Size: 0x40 (Inherited: 0x38)
struct FScalarMaterialInput : FMaterialInput {
	char UseConstant : 1; // 0x38(0x01)
	char UnknownData_38_1 : 7; // 0x38(0x01)
	char UnknownData_39[0x3]; // 0x39(0x03)
	float Constant; // 0x3c(0x04)
};
// ScriptStruct Engine.VectorMaterialInput
// Size: 0x48 (Inherited: 0x38)
struct FVectorMaterialInput : FMaterialInput {
	char UseConstant : 1; // 0x38(0x01)
	char UnknownData_38_1 : 7; // 0x38(0x01)
	char UnknownData_39[0x3]; // 0x39(0x03)
	struct FVector Constant; // 0x3c(0x0c)
};
// ScriptStruct Engine.Vector2MaterialInput
// Size: 0x48 (Inherited: 0x38)
struct FVector2MaterialInput : FMaterialInput {
	char UseConstant : 1; // 0x38(0x01)
	char UnknownData_38_1 : 7; // 0x38(0x01)
	char UnknownData_39[0x3]; // 0x39(0x03)
	float ConstantX; // 0x3c(0x04)
	float ConstantY; // 0x40(0x04)
	char UnknownData_44[0x4]; // 0x44(0x04)
};
// ScriptStruct Engine.ExpressionInput
// Size: 0x38 (Inherited: 0x00)
struct FExpressionInput {
	struct UMaterialExpression* Expression; // 0x00(0x08)
	int32_t OutputIndex; // 0x08(0x04)
	char UnknownData_C[0x4]; // 0x0c(0x04)
	struct FString InputName; // 0x10(0x10)
	int32_t Mask; // 0x20(0x04)
	int32_t MaskR; // 0x24(0x04)
	int32_t MaskG; // 0x28(0x04)
	int32_t MaskB; // 0x2c(0x04)
	int32_t MaskA; // 0x30(0x04)
	int32_t GCC64_Padding; // 0x34(0x04)
};
// Class Engine.PrimitiveComponent
// Size: 0x5c0 (Inherited: 0x2e0)
struct UPrimitiveComponent : USceneComponent {
	char UnknownData_2E0[0x8]; // 0x2e0(0x08)
	float MinDrawDistance; // 0x2e8(0x04)
	char UnknownData_2EC[0x4]; // 0x2ec(0x04)
	float LDMaxDrawDistance; // 0x2f0(0x04)
	float CachedMaxDrawDistance; // 0x2f4(0x04)
	char DepthPriorityGroup; // 0x2f8(0x01)
	char ViewOwnerDepthPriorityGroup; // 0x2f9(0x01)
	char UnknownData_2FA[0x2]; // 0x2fa(0x02)
	char bAlwaysCreatePhysicsState : 1; // 0x2fc(0x01)
	char bGenerateOverlapEvents : 1; // 0x2fc(0x01)
	char bMultiBodyOverlap : 1; // 0x2fc(0x01)
	char bCheckAsyncSceneOnMove : 1; // 0x2fc(0x01)
	char bTraceComplexOnMove : 1; // 0x2fc(0x01)
	char bReturnMaterialOnMove : 1; // 0x2fc(0x01)
	char bUseViewOwnerDepthPriorityGroup : 1; // 0x2fc(0x01)
	char bAllowCullDistanceVolume : 1; // 0x2fc(0x01)
	char bHasMotionBlurVelocityMeshes : 1; // 0x2fd(0x01)
	char bRenderInMainPass : 1; // 0x2fd(0x01)
	char bRenderInCustomPrePass : 1; // 0x2fd(0x01)
	char bReflected : 1; // 0x2fd(0x01)
	char UnknownData_2FD_4 : 1; // 0x2fd(0x01)
	char bReflectedOnLowQuality : 1; // 0x2fd(0x01)
	char bFFTWaterMask : 1; // 0x2fd(0x01)
	char bVolumeFogMask : 1; // 0x2fd(0x01)
	char UnknownData_2FE_0 : 1; // 0x2fe(0x01)
	char bAffectsFlatWater : 1; // 0x2fe(0x01)
	char bGPUParticlesKillPlane : 1; // 0x2fe(0x01)
	char bDontCull : 1; // 0x2fe(0x01)
	char bDontSizeOnScreenCull : 1; // 0x2fe(0x01)
	char UnknownData_2FE_5 : 3; // 0x2fe(0x01)
	char UnknownData_2FF[0x1]; // 0x2ff(0x01)
	float OverriddenShadowMinTexelSize; // 0x300(0x04)
	bool bOverrideShadowMinSizeCulling; // 0x304(0x01)
	bool bOverrideShadowCascadesExclusion; // 0x305(0x01)
	char ExcludedShadowCascades; // 0x306(0x01)
	char UnknownData_307[0x1]; // 0x307(0x01)
	char bReceivesDecals : 1; // 0x308(0x01)
	char bOwnerNoSee : 1; // 0x308(0x01)
	char bOnlyOwnerSee : 1; // 0x308(0x01)
	char bTreatAsBackgroundForOcclusion : 1; // 0x308(0x01)
	char bIsACloud : 1; // 0x308(0x01)
	char bUseAsOccluder : 1; // 0x308(0x01)
	char bSelectable : 1; // 0x308(0x01)
	char bForceMipStreaming : 1; // 0x308(0x01)
	char bHasPerInstanceHitProxies : 1; // 0x309(0x01)
	char CastShadow : 1; // 0x309(0x01)
	char bAffectDynamicIndirectLighting : 1; // 0x309(0x01)
	char bUseFarCascadeLPVBiasMultiplier : 1; // 0x309(0x01)
	char bAffectDistanceFieldLighting : 1; // 0x309(0x01)
	char bCastDynamicShadow : 1; // 0x309(0x01)
	char bCastStaticShadow : 1; // 0x309(0x01)
	char bCastVolumetricTranslucentShadow : 1; // 0x309(0x01)
	char bSelfShadowOnly : 1; // 0x30a(0x01)
	char bCastFarShadow : 1; // 0x30a(0x01)
	char bCastInsetShadow : 1; // 0x30a(0x01)
	char bCastCinematicShadow : 1; // 0x30a(0x01)
	char bCastHiddenShadow : 1; // 0x30a(0x01)
	char bCastShadowAsTwoSided : 1; // 0x30a(0x01)
	char bCastShadowOnLowQuality : 1; // 0x30a(0x01)
	char bLightAsIfStatic : 1; // 0x30a(0x01)
	char bLightAttachmentsAsGroup : 1; // 0x30b(0x01)
	char UnknownData_30B_1 : 7; // 0x30b(0x01)
	char IndirectLightingCacheQuality; // 0x30c(0x01)
	bool bHasCachedStaticLighting; // 0x30d(0x01)
	bool bStaticLightingBuildEnqueued; // 0x30e(0x01)
	char UnknownData_30F[0x1]; // 0x30f(0x01)
	char bIgnoreRadialImpulse : 1; // 0x310(0x01)
	char bIgnoreRadialForce : 1; // 0x310(0x01)
	char AlwaysLoadOnClient : 1; // 0x310(0x01)
	char AlwaysLoadOnServer : 1; // 0x310(0x01)
	char bUseEditorCompositing : 1; // 0x310(0x01)
	char bRenderCustomDepth : 1; // 0x310(0x01)
	char bAllowVelocityInMaterial : 1; // 0x310(0x01)
	char UnknownData_311[0x3]; // 0x311(0x03)
	int32_t CustomDepthStencilValue; // 0x314(0x04)
	int32_t TranslucencySortPriority; // 0x318(0x04)
	int32_t VisibilityId; // 0x31c(0x04)
	char pad1[0x2A0];

	void WakeRigidBody(struct FName BoneName); // Function Engine.PrimitiveComponent.WakeRigidBody // Native|Public|BlueprintCallable // @ game+0x2d18570
	void WakeAllRigidBodies(); // Function Engine.PrimitiveComponent.WakeAllRigidBodies // Native|Public|BlueprintCallable // @ game+0x2d18550
	void SetWalkableSlopeOverride(struct FWalkableSlopeOverride NewOverride); // Function Engine.PrimitiveComponent.SetWalkableSlopeOverride // Final|Native|Public|HasOutParms|BlueprintCallable // @ game+0x2d17440
	void SetTranslucentSortPriority(int32_t NewTranslucentSortPriority); // Function Engine.PrimitiveComponent.SetTranslucentSortPriority // Final|Native|Public|BlueprintCallable // @ game+0x2d17100
	void SetSimulatePhysics(bool bSimulate); // Function Engine.PrimitiveComponent.SetSimulatePhysics // Native|Public|BlueprintCallable // @ game+0x2d16820
	void SetRenderInMainPass(bool bValue); // Function Engine.PrimitiveComponent.SetRenderInMainPass // Final|Native|Public|BlueprintCallable // @ game+0x2d165e0
	void SetRenderCustomDepth(bool bValue); // Function Engine.PrimitiveComponent.SetRenderCustomDepth // Final|Native|Public|BlueprintCallable // @ game+0x2d16550
	void SetPhysMaterialOverride(struct UPhysicalMaterial* NewPhysMaterial); // Function Engine.PrimitiveComponent.SetPhysMaterialOverride // Native|Public|BlueprintCallable // @ game+0x2d15a90
	void SetPhysicsMaxAngularVelocity(float NewMaxAngVel, bool bAddToCurrent, struct FName BoneName); // Function Engine.PrimitiveComponent.SetPhysicsMaxAngularVelocity // Final|Native|Public|BlueprintCallable // @ game+0x2d15e90
	void SetPhysicsLinearVelocity(struct FVector NewVel, bool bAddToCurrent, struct FName BoneName); // Function Engine.PrimitiveComponent.SetPhysicsLinearVelocity // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d15d80
	void SetPhysicsAngularVelocity(struct FVector NewAngVel, bool bAddToCurrent, struct FName BoneName); // Function Engine.PrimitiveComponent.SetPhysicsAngularVelocity // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d15b20
	void SetOwnerNoSee(bool bNewOwnerNoSee); // Function Engine.PrimitiveComponent.SetOwnerNoSee // Final|Native|Public|BlueprintCallable // @ game+0x2d15a00
	void SetOnlyOwnerSee(bool bNewOnlyOwnerSee); // Function Engine.PrimitiveComponent.SetOnlyOwnerSee // Final|Native|Public|BlueprintCallable // @ game+0x2d158f0
	void SetNotifyRigidBodyCollision(bool bNewNotifyRigidBodyCollision); // Function Engine.PrimitiveComponent.SetNotifyRigidBodyCollision // Native|Public|BlueprintCallable // @ game+0x2d15860
	void SetMaterial(int32_t ElementIndex, struct UMaterialInterface* Material)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PrimitiveComponent.SetMaterial");
		struct
		{
			int32_t Eindex = 0;
			struct UMaterialInterface* Mterial = nullptr;
		}params;
		ElementIndex = params.Eindex;
		Material = params.Mterial;
		ProcessEvent(this, fn, &params);
	}; // Function Engine.PrimitiveComponent.SetMaterial // Native|Public|BlueprintCallable // @ game+0x2d15480
	void SetMassScale(struct FName BoneName, float InMassScale); // Function Engine.PrimitiveComponent.SetMassScale // Native|Public|BlueprintCallable // @ game+0x2d15330
	void SetMassOverrideInKg(struct FName BoneName, float MassInKg, bool bOverrideMass); // Function Engine.PrimitiveComponent.SetMassOverrideInKg // Native|Public|BlueprintCallable // @ game+0x2d15230
	void SetLockedAxis(char LockedAxis); // Function Engine.PrimitiveComponent.SetLockedAxis // Native|Public|BlueprintCallable // @ game+0x2d15120
	void SetLinearDamping(float InDamping); // Function Engine.PrimitiveComponent.SetLinearDamping // Native|Public|BlueprintCallable // @ game+0x2d148e0
	void SetGenerateOverlapEvents(bool bEnable); // Function Engine.PrimitiveComponent.SetGenerateOverlapEvents // Final|Native|Public|BlueprintCallable // @ game+0x2d14060
	void SetEnableGravity(bool bGravityEnabled); // Function Engine.PrimitiveComponent.SetEnableGravity // Native|Public|BlueprintCallable // @ game+0x2d13d10
	void SetCustomPrimitiveVector(int32_t Index, struct FVector Value); // Function Engine.PrimitiveComponent.SetCustomPrimitiveVector // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d138f0
	void SetCustomPrimitiveScalar(int32_t Index, float Value); // Function Engine.PrimitiveComponent.SetCustomPrimitiveScalar // Final|Native|Public|BlueprintCallable // @ game+0x2d13820
	void SetCustomDepthStencilValue(int32_t Value); // Function Engine.PrimitiveComponent.SetCustomDepthStencilValue // Final|Native|Public|BlueprintCallable // @ game+0x2d13640
	void SetCullDistance(float NewCullDistance); // Function Engine.PrimitiveComponent.SetCullDistance // Final|Native|Public|BlueprintCallable // @ game+0x2d13540
	void SetConstraintMode(char ConstraintMode); // Function Engine.PrimitiveComponent.SetConstraintMode // Native|Public|BlueprintCallable // @ game+0x2d131b0
	void SetCollisionResponseToChannel(char Channel, char NewResponse); // Function Engine.PrimitiveComponent.SetCollisionResponseToChannel // Final|Native|Public|BlueprintCallable // @ game+0x2d12e00
	void SetCollisionResponseToAllChannels(char NewResponse); // Function Engine.PrimitiveComponent.SetCollisionResponseToAllChannels // Final|Native|Public|BlueprintCallable // @ game+0x2d12d80
	void SetCollisionProfileName(struct FName InCollisionProfileName); // Function Engine.PrimitiveComponent.SetCollisionProfileName // Native|Public|BlueprintCallable // @ game+0x2d12cf0
	void SetCollisionObjectType(char Channel); // Function Engine.PrimitiveComponent.SetCollisionObjectType // Final|Native|Public|BlueprintCallable // @ game+0x2d12c70
	void SetCollisionEnabled(char NewType); // Function Engine.PrimitiveComponent.SetCollisionEnabled // Native|Public|BlueprintCallable // @ game+0x2d12bf0
	void SetCenterOfMass(struct FVector CenterOfMassOffset, struct FName BoneName); // Function Engine.PrimitiveComponent.SetCenterOfMass // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d12850
	void SetCastShadow(bool NewCastShadow); // Function Engine.PrimitiveComponent.SetCastShadow // Final|Native|Public|BlueprintCallable // @ game+0x2d126a0
	void SetAngularDamping(float InDamping); // Function Engine.PrimitiveComponent.SetAngularDamping // Native|Public|BlueprintCallable // @ game+0x2d11480
	void SetAllPhysicsLinearVelocity(struct FVector NewVel, bool bAddToCurrent); // Function Engine.PrimitiveComponent.SetAllPhysicsLinearVelocity // Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d113a0
	void SetAllMassScale(float InMassScale); // Function Engine.PrimitiveComponent.SetAllMassScale // Native|Public|BlueprintCallable // @ game+0x2d10f80
	struct FVector ScaleByMomentOfInertia(struct FVector InputVector, struct FName BoneName); // Function Engine.PrimitiveComponent.ScaleByMomentOfInertia // Native|Public|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2d10800
	void PutRigidBodyToSleep(struct FName BoneName); // Function Engine.PrimitiveComponent.PutRigidBodyToSleep // Final|Native|Public|BlueprintCallable // @ game+0x2d101d0
	bool K2_LineTraceComponent(struct FVector TraceStart, struct FVector TraceEnd, bool bTraceComplex, bool bShowTrace, struct FVector HitLocation, struct FVector HitNormal, struct FName BoneName); // Function Engine.PrimitiveComponent.K2_LineTraceComponent // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable // @ game+0x2d0e6e0
	bool IsOverlappingComponent(struct UPrimitiveComponent* OtherComp); // Function Engine.PrimitiveComponent.IsOverlappingComponent // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0e2f0
	bool IsOverlappingActor(struct AActor* Other); // Function Engine.PrimitiveComponent.IsOverlappingActor // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0e260
	bool IsGravityEnabled(); // Function Engine.PrimitiveComponent.IsGravityEnabled // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0e090
	void IgnoreActorWhenMoving(struct AActor* Actor, bool bShouldIgnore); // Function Engine.PrimitiveComponent.IgnoreActorWhenMoving // Final|Native|Public|BlueprintCallable // @ game+0x2d0dc70
	struct FWalkableSlopeOverride GetWalkableSlopeOverride(); // Function Engine.PrimitiveComponent.GetWalkableSlopeOverride // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0d4a0
	struct FVector GetPhysicsLinearVelocityAtPoint(struct FVector Point, struct FName BoneName); // Function Engine.PrimitiveComponent.GetPhysicsLinearVelocityAtPoint // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d0b190
	struct FVector GetPhysicsLinearVelocity(struct FName BoneName); // Function Engine.PrimitiveComponent.GetPhysicsLinearVelocity // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d0b0f0
	struct FVector GetPhysicsAngularVelocity(struct FName BoneName); // Function Engine.PrimitiveComponent.GetPhysicsAngularVelocity // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d0b050
	void GetOverlappingComponents(struct TArray<struct UPrimitiveComponent*> InOverlappingComponents); // Function Engine.PrimitiveComponent.GetOverlappingComponents // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0a990
	void GetOverlappingActors(struct TArray<struct AActor*> OverlappingActors, struct UClass* ClassFilter); // Function Engine.PrimitiveComponent.GetOverlappingActors // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0a890
	struct TArray<struct FOverlapInfo> GetOverlapInfos(); // Function Engine.PrimitiveComponent.GetOverlapInfos // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0a670
	int32_t GetNumMaterials(); // Function Engine.PrimitiveComponent.GetNumMaterials // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0a5b0
	struct UMaterialInterface* GetMaterial(int32_t ElementIndex); // Function Engine.PrimitiveComponent.GetMaterial // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0a0e0
	float GetMassScale(struct FName BoneName); // Function Engine.PrimitiveComponent.GetMassScale // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0a020
	float GetMass(); // Function Engine.PrimitiveComponent.GetMass // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d09fe0
	float GetLinearDamping(); // Function Engine.PrimitiveComponent.GetLinearDamping // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d09a10
	struct FVector GetInertiaTensor(struct FName BoneName); // Function Engine.PrimitiveComponent.GetInertiaTensor // Native|Public|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2d097d0
	char GetCollisionResponseToChannel(char Channel); // Function Engine.PrimitiveComponent.GetCollisionResponseToChannel // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d08f80
	struct FName GetCollisionProfileName(); // Function Engine.PrimitiveComponent.GetCollisionProfileName // Final|Native|Public|BlueprintCallable // @ game+0x2d08f40
	char GetCollisionObjectType(); // Function Engine.PrimitiveComponent.GetCollisionObjectType // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d08f10
	float GetClosestPointOnCollision(struct FVector Point, struct FVector OutPointOnBody, struct FName BoneName); // Function Engine.PrimitiveComponent.GetClosestPointOnCollision // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2d08da0
	struct FVector GetCenterOfMass(struct FName BoneName); // Function Engine.PrimitiveComponent.GetCenterOfMass // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d08d00
	float GetAngularDamping(); // Function Engine.PrimitiveComponent.GetAngularDamping // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d08a60
	struct UMaterialInstanceDynamic* CreateDynamicMaterialInstance(int32_t ElementIndex, struct UMaterialInterface* SourceMaterial); // Function Engine.PrimitiveComponent.CreateDynamicMaterialInstance // Native|Public|BlueprintCallable // @ game+0x2d067c0
	struct UMaterialInstanceDynamic* CreateAndSetMaterialInstanceDynamicFromMaterial(int32_t ElementIndex, struct UMaterialInterface* Parent); // Function Engine.PrimitiveComponent.CreateAndSetMaterialInstanceDynamicFromMaterial // Native|Public|BlueprintCallable // @ game+0x2d066c0
	struct UMaterialInstanceDynamic* CreateAndSetMaterialInstanceDynamic(int32_t ElementIndex); // Function Engine.PrimitiveComponent.CreateAndSetMaterialInstanceDynamic // Native|Public|BlueprintCallable // @ game+0x2d06620
	struct TArray<struct AActor*> CopyArrayOfMoveIgnoreActors(); // Function Engine.PrimitiveComponent.CopyArrayOfMoveIgnoreActors // Final|Native|Public|BlueprintCallable // @ game+0x2d06580
	void ClearMoveIgnoreActors(int32_t InSlack); // Function Engine.PrimitiveComponent.ClearMoveIgnoreActors // Final|Native|Public|BlueprintCallable // @ game+0x2d06160
	bool CanCharacterStepUp(struct APawn* Pawn); // Function Engine.PrimitiveComponent.CanCharacterStepUp // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d05d80
	void AddTorque(struct FVector Torque, struct FName BoneName, bool bAccelChange); // Function Engine.PrimitiveComponent.AddTorque // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d05a10
	void AddRadialImpulse(struct FVector Origin, float Radius, float Strength, char Falloff, bool bVelChange); // Function Engine.PrimitiveComponent.AddRadialImpulse // Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d05650
	void AddRadialForce(struct FVector Origin, float Radius, float Strength, char Falloff, bool bAccelChange); // Function Engine.PrimitiveComponent.AddRadialForce // Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d05490
	void AddImpulseAtLocation(struct FVector Impulse, struct FVector Location, struct FName BoneName); // Function Engine.PrimitiveComponent.AddImpulseAtLocation // Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d050c0
	void AddImpulse(struct FVector Impulse, struct FName BoneName, bool bVelChange); // Function Engine.PrimitiveComponent.AddImpulse // Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d04fa0
	void AddForceAtLocation(struct FVector Force, struct FVector Location, struct FName BoneName); // Function Engine.PrimitiveComponent.AddForceAtLocation // Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d04ce0
	void AddForce(struct FVector Force, struct FName BoneName, bool bAccelChange); // Function Engine.PrimitiveComponent.AddForce // Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d04bc0
	void AddAngularImpulse(struct FVector Impulse, struct FName BoneName, bool bVelChange); // Function Engine.PrimitiveComponent.AddAngularImpulse // Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d043c0
};
// ScriptStruct Engine.MaterialAttributesInput
// Size: 0x38 (Inherited: 0x38)
struct FMaterialAttributesInput : FExpressionInput {
};

// ScriptStruct CoreUObject.BoxSphereBounds
// Size: 0x1c (Inherited: 0x00)
struct FBoxSphereBounds {
	struct FVector Origin; // 0x00(0x0c)
	struct FVector BoxExtent; // 0x0c(0x0c)
	float SphereRadius; // 0x18(0x04)
};
// Class Engine.MeshComponent
// Size: 0x5d0 (Inherited: 0x5c0)
struct UMeshComponent : UPrimitiveComponent {
	struct TArray<struct UMaterialInterface*> OverrideMaterials; // 0x5b8(0x10)

	void ResetDefaultMaterials(); // Function Engine.MeshComponent.ResetDefaultMaterials // Final|Native|Public|BlueprintCallable // @ game+0x2d106c0
	struct TArray<struct UMaterialInterface*> GetMaterials()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.MeshComponent.GetMaterials");
		bool yes = true;
		ProcessEvent(this, fn, &yes);
	}; // Function Engine.MeshComponent.GetMaterials // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0a180
};
// Class Engine.SkinnedMeshComponent
// Size: 0x6f0 (Inherited: 0x5d0)
struct USkinnedMeshComponent : UMeshComponent {
	struct USkeletalMesh* SkeletalMesh; // 0x5c8(0x08)
	struct USkinnedMeshComponent* MasterPoseComponent; // 0x5d0(0x08)
	char UnknownData_5E0[0x38]; // 0x5e0(0x38)
	struct UMaterialInterface* CustomDepthMaterial; // 0x618(0x08)
	char UnknownData_620[0x10]; // 0x620(0x10)
	char bUseBoundsFromMasterPoseComponent : 1; // 0x630(0x01)
	char UnknownData_630_1 : 7; // 0x630(0x01)
	char UnknownData_631[0x7]; // 0x631(0x07)
	struct TArray<struct FActiveVertexAnim> ActiveVertexAnims; // 0x638(0x10)
	struct UPhysicsAsset* PhysicsAssetOverride; // 0x648(0x08)
	int32_t ForcedLodModel; // 0x650(0x04)
	int32_t MinLodModel; // 0x654(0x04)
	int32_t PredictedLODLevel; // 0x658(0x04)
	int32_t OldPredictedLODLevel; // 0x65c(0x04)
	float MaxDistanceFactor; // 0x660(0x04)
	char UnknownData_664[0x4]; // 0x664(0x04)
	struct TArray<struct FSkelMeshComponentLODInfo> LODInfo; // 0x668(0x10)
	float StreamingDistanceMultiplier; // 0x678(0x04)
	struct FColor WireframeColor; // 0x67c(0x04)
	char bForceWireframe : 1; // 0x680(0x01)
	char bDisplayBones : 1; // 0x680(0x01)
	char bDisplayBoneCollisions : 1; // 0x680(0x01)
	char bDisableMorphTarget : 1; // 0x680(0x01)
	char bHideSkin : 1; // 0x680(0x01)
	char UnknownData_680_5 : 3; // 0x680(0x01)
	char UnknownData_681[0x17]; // 0x681(0x17)
	char bPerBoneMotionBlur : 1; // 0x698(0x01)
	char bComponentUseFixedSkelBounds : 1; // 0x698(0x01)
	char bConsiderAllBodiesForBounds : 1; // 0x698(0x01)
	char UnknownData_698_3 : 5; // 0x698(0x01)
	char UnknownData_699[0x3]; // 0x699(0x03)
	char MeshComponentUpdateFlag; // 0x69c(0x01)
	char UnknownData_69D[0x3]; // 0x69d(0x03)
	char bForceMeshObjectUpdate : 1; // 0x6a0(0x01)
	char bChartDistanceFactor : 1; // 0x6a0(0x01)
	char bCanHighlightSelectedSections : 1; // 0x6a0(0x01)
	char bRecentlyRendered : 1; // 0x6a0(0x01)
	char UnknownData_6A0_4 : 4; // 0x6a0(0x01)
	char UnknownData_6A1[0x3]; // 0x6a1(0x03)
	float ProgressiveDrawingFraction; // 0x6a4(0x04)
	char CustomSortAlternateIndexMode; // 0x6a8(0x01)
	char UnknownData_6A9[0x13]; // 0x6a9(0x13)
	struct FBoxSphereBounds CachedLocalBounds; // 0x6bc(0x1c)
	bool bCachedLocalBoundsUpToDate; // 0x6d8(0x01)
	bool bEnableUpdateRateOptimizations; // 0x6d9(0x01)
	bool bDisplayDebugUpdateRateOptimizations; // 0x6da(0x01)
	char UnknownData_6DB[0x15]; // 0x6db(0x15)

	void UnHideBoneByName(struct FName BoneName); // Function Engine.SkinnedMeshComponent.UnHideBoneByName // Final|Native|Public|BlueprintCallable // @ game+0x2d18280
	void TransformToBoneSpace(struct FName BoneName, struct FVector InPosition, struct FRotator InRotation, struct FVector OutPosition, struct FRotator OutRotation); // Function Engine.SkinnedMeshComponent.TransformToBoneSpace // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2d17f30
	void TransformFromBoneSpace(struct FName BoneName, struct FVector InPosition, struct FRotator InRotation, struct FVector OutPosition, struct FRotator OutRotation); // Function Engine.SkinnedMeshComponent.TransformFromBoneSpace // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable // @ game+0x2d17d70
	void SetSkeletalMesh(struct USkeletalMesh* NewMesh); // Function Engine.SkinnedMeshComponent.SetSkeletalMesh // Native|Public|BlueprintCallable // @ game+0x2d168b0
	void SetPhysicsAsset(struct UPhysicsAsset* NewPhysicsAsset, bool bForceReInit); // Function Engine.SkinnedMeshComponent.SetPhysicsAsset // Native|Public|BlueprintCallable // @ game+0x2d15c30
	void SetMasterPoseComponent(struct USkinnedMeshComponent* NewMasterBoneComponent); // Function Engine.SkinnedMeshComponent.SetMasterPoseComponent // Final|Native|Public|BlueprintCallable // @ game+0x2d15400
	void SetCustomDepthMaterial(struct UMaterialInterface* Material); // Function Engine.SkinnedMeshComponent.SetCustomDepthMaterial // Final|Native|Public|BlueprintCallable // @ game+0x2d135c0
	void OnRep_SkeletalMesh(struct USkeletalMesh* OldMesh); // Function Engine.SkinnedMeshComponent.OnRep_SkeletalMesh // Final|Native|Public // @ game+0x2d0f630
	bool IsBoneHiddenByName(struct FName BoneName); // Function Engine.SkinnedMeshComponent.IsBoneHiddenByName // Final|Native|Public|BlueprintCallable // @ game+0x2d0de80
	void HideBoneByName(struct FName BoneName, char PhysBodyOption); // Function Engine.SkinnedMeshComponent.HideBoneByName // Final|Native|Public|BlueprintCallable // @ game+0x2d0dbb0
	struct FName GetSocketBoneName(struct FName InSocketName); // Function Engine.SkinnedMeshComponent.GetSocketBoneName // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0c790
	struct FName GetParentBone(struct FName BoneName); // Function Engine.SkinnedMeshComponent.GetParentBone // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0aaa0
	struct UMaterialInterface* GetCustomDepthMaterial(); // Function Engine.SkinnedMeshComponent.GetCustomDepthMaterial // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d09250
	struct FName GetBoneName(int32_t BoneIndex); // Function Engine.SkinnedMeshComponent.GetBoneName // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d08c40
	int32_t GetBoneIndex(struct FName BoneName); // Function Engine.SkinnedMeshComponent.GetBoneIndex // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d08bb0
	bool BoneIsChildOf(struct FName BoneName, struct FName ParentBoneName); // Function Engine.SkinnedMeshComponent.BoneIsChildOf // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d05ca0
};
// ScriptStruct Engine.SingleAnimationPlayData
// Size: 0x20 (Inherited: 0x00)
struct FSingleAnimationPlayData {
	struct UAnimationAsset* AnimToPlay; // 0x00(0x08)
	struct UVertexAnimation* VertexAnimToPlay; // 0x08(0x08)
	char bSavedLooping : 1; // 0x10(0x01)
	char bSavedPlaying : 1; // 0x10(0x01)
	char UnknownData_10_2 : 6; // 0x10(0x01)
	char UnknownData_11[0x3]; // 0x11(0x03)
	float SavedPosition; // 0x14(0x04)
	float SavedPlayRate; // 0x18(0x04)
	char UnknownData_1C[0x4]; // 0x1c(0x04)
};

// Class Engine.SkeletalMeshComponent
// Size: 0x9c0 (Inherited: 0x6f0)
struct USkeletalMeshComponent : USkinnedMeshComponent {
	char UnknownData_6F0[0x10]; // 0x6f0(0x10)
	char AnimationMode; // 0x700(0x01)
	char UnknownData_701[0x7]; // 0x701(0x07)
	struct UClass* AnimBlueprintGeneratedClass; // 0x708(0x08)
	struct UAnimInstance* AnimScriptInstance; // 0x710(0x08)
	struct FSingleAnimationPlayData AnimationData; // 0x718(0x20)
	char UnknownData_738[0x10]; // 0x738(0x10)
	struct TArray<struct FTransform> CachedLocalAtoms; // 0x748(0x10)
	struct TArray<struct FTransform> CachedSpaceBases; // 0x758(0x10)
	char UnknownData_768[0x28]; // 0x768(0x28)
	float GlobalAnimRateScale; // 0x790(0x04)
	char bHasValidBodies : 1; // 0x794(0x01)
	char UnknownData_794_1 : 7; // 0x794(0x01)
	char UnknownData_795[0x3]; // 0x795(0x03)
	char KinematicBonesUpdateType; // 0x798(0x01)
	char UnknownData_799[0x3]; // 0x799(0x03)
	char bBlendPhysics : 1; // 0x79c(0x01)
	char bEnablePhysicsOnDedicatedServer : 1; // 0x79c(0x01)
	char bUpdateJointsFromAnimation : 1; // 0x79c(0x01)
	char bDisableClothSimulation : 1; // 0x79c(0x01)
	char bCollideWithEnvironment : 1; // 0x79c(0x01)
	char bCollideWithAttachedChildren : 1; // 0x79c(0x01)
	char bLocalSpaceSimulation : 1; // 0x79c(0x01)
	char bClothMorphTarget : 1; // 0x79c(0x01)
	char bResetAfterTeleport : 1; // 0x79d(0x01)
	char UnknownData_79D_1 : 7; // 0x79d(0x01)
	char UnknownData_79E[0x2]; // 0x79e(0x02)
	float TeleportDistanceThreshold; // 0x7a0(0x04)
	float TeleportRotationThreshold; // 0x7a4(0x04)
	float ClothBlendWeight; // 0x7a8(0x04)
	char UnknownData_7AC[0x4]; // 0x7ac(0x04)
	struct FVector RootBoneTranslation; // 0x7b0(0x0c)
	char bNoSkeletonUpdate : 1; // 0x7bc(0x01)
	char bPauseAnims : 1; // 0x7bc(0x01)
	char bEnablePerPolyCollision : 1; // 0x7bc(0x01)
	char UnknownData_7BC_3 : 5; // 0x7bc(0x01)
	char UnknownData_7BD[0x3]; // 0x7bd(0x03)
	struct UBodySetup* BodySetup; // 0x7c0(0x08)
	char bAutonomousTickPose : 1; // 0x7c8(0x01)
	char bForceRefpose : 1; // 0x7c8(0x01)
	char bOldForceRefPose : 1; // 0x7c8(0x01)
	char bShowPrePhysBones : 1; // 0x7c8(0x01)
	char bRequiredBonesUpToDate : 1; // 0x7c8(0x01)
	char bAnimTreeInitialised : 1; // 0x7c8(0x01)
	char bEnableLineCheckWithBounds : 1; // 0x7c8(0x01)
	char UnknownData_7C9[0x3]; // 0x7c9(0x03)
	struct FVector LineCheckBoundsScale; // 0x7cc(0x0c)
	char pad1[0x1E8];

	void UnbindClothFromMasterPoseComponent(bool bRestoreSimulationSpace); // Function Engine.SkeletalMeshComponent.UnbindClothFromMasterPoseComponent // Final|Native|Public|BlueprintCallable // @ game+0x2d18300
	void Stop(); // Function Engine.SkeletalMeshComponent.Stop // Final|Native|Public|BlueprintCallable // @ game+0x2d17b00
	void SetPosition(float InPos, bool bFireNotifies); // Function Engine.SkeletalMeshComponent.SetPosition // Final|Native|Public|BlueprintCallable // @ game+0x2d16400
	void SetPlayRate(float Rate); // Function Engine.SkeletalMeshComponent.SetPlayRate // Final|Native|Public|BlueprintCallable // @ game+0x2d162b0
	void SetPhysicsBlendWeight(float PhysicsBlendWeight); // Function Engine.SkeletalMeshComponent.SetPhysicsBlendWeight // Final|Native|Public|BlueprintCallable // @ game+0x2d15d00
	void SetMorphTarget(struct FName MorphTargetName, float Value, bool bRemoveZeroWeight); // Function Engine.SkeletalMeshComponent.SetMorphTarget // Final|Native|Public|BlueprintCallable // @ game+0x2d15750
	void SetEnablePhysicsBlending(bool bNewBlendPhysics); // Function Engine.SkeletalMeshComponent.SetEnablePhysicsBlending // Final|Native|Public|BlueprintCallable // @ game+0x2d13e30
	void SetClothMaxDistanceScale(float Scale); // Function Engine.SkeletalMeshComponent.SetClothMaxDistanceScale // Final|Native|Public|BlueprintCallable // @ game+0x2d12b70
	void SetAnimInstanceClass(struct UClass* NewClass); // Function Engine.SkeletalMeshComponent.SetAnimInstanceClass // Final|Native|Public|BlueprintCallable // @ game+0x2d11b10
	void SetAnimationMode(char InAnimationMode); // Function Engine.SkeletalMeshComponent.SetAnimationMode // Final|Native|Public|BlueprintCallable // @ game+0x2d11c10
	void SetAnimation(struct UAnimationAsset* NewAnimToPlay); // Function Engine.SkeletalMeshComponent.SetAnimation // Final|Native|Public|BlueprintCallable // @ game+0x2d11b90
	void SetAllMotorsAngularVelocityDrive(bool bEnableSwingDrive, bool bEnableTwistDrive, bool bSkipCustomPhysicsType); // Function Engine.SkeletalMeshComponent.SetAllMotorsAngularVelocityDrive // Final|Native|Public|BlueprintCallable // @ game+0x2d11280
	void SetAllMotorsAngularPositionDrive(bool bEnableSwingDrive, bool bEnableTwistDrive, bool bSkipCustomPhysicsType); // Function Engine.SkeletalMeshComponent.SetAllMotorsAngularPositionDrive // Final|Native|Public|BlueprintCallable // @ game+0x2d11160
	void SetAllMotorsAngularDriveParams(float InSpring, float InDamping, float InForceLimit, bool bSkipCustomPhysicsType); // Function Engine.SkeletalMeshComponent.SetAllMotorsAngularDriveParams // Final|Native|Public|BlueprintCallable // @ game+0x2d11000
	void SetAllBodiesSimulatePhysics(bool bNewSimulate); // Function Engine.SkeletalMeshComponent.SetAllBodiesSimulatePhysics // Final|Native|Public|BlueprintCallable // @ game+0x2d10ef0
	void SetAllBodiesPhysicsBlendWeight(float PhysicsBlendWeight, bool bSkipCustomPhysicsType); // Function Engine.SkeletalMeshComponent.SetAllBodiesPhysicsBlendWeight // Final|Native|Public|BlueprintCallable // @ game+0x2d10e20
	void SetAllBodiesBelowSimulatePhysics(struct FName InBoneName, bool bNewSimulate); // Function Engine.SkeletalMeshComponent.SetAllBodiesBelowSimulatePhysics // Final|Native|Public|HasOutParms|BlueprintCallable // @ game+0x2d10d50
	void SetAllBodiesBelowPhysicsBlendWeight(struct FName InBoneName, float PhysicsBlendWeight, bool bSkipCustomPhysicsType); // Function Engine.SkeletalMeshComponent.SetAllBodiesBelowPhysicsBlendWeight // Final|Native|Public|HasOutParms|BlueprintCallable // @ game+0x2d10c30
	void ResetClothTeleportMode(); // Function Engine.SkeletalMeshComponent.ResetClothTeleportMode // Final|Native|Public|BlueprintCallable // @ game+0x2d106a0
	void ResetAllBodiesSimulatePhysics(); // Function Engine.SkeletalMeshComponent.ResetAllBodiesSimulatePhysics // Final|Native|Public|BlueprintCallable // @ game+0x2d10680
	void PlayAnimation(struct UAnimationAsset* NewAnimToPlay, bool bLooping); // Function Engine.SkeletalMeshComponent.PlayAnimation // Final|Native|Public|BlueprintCallable // @ game+0x2d0fdc0
	void Play(bool bLooping); // Function Engine.SkeletalMeshComponent.Play // Final|Native|Public|BlueprintCallable // @ game+0x2d0fc20
	bool IsPlaying(); // Function Engine.SkeletalMeshComponent.IsPlaying // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0e380
	float GetPosition(); // Function Engine.SkeletalMeshComponent.GetPosition // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0b380
	float GetPlayRate(); // Function Engine.SkeletalMeshComponent.GetPlayRate // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0b350
	float GetMorphTarget(struct FName MorphTargetName); // Function Engine.SkeletalMeshComponent.GetMorphTarget // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0a260
	float GetClothMaxDistanceScale(); // Function Engine.SkeletalMeshComponent.GetClothMaxDistanceScale // Final|Native|Public|BlueprintCallable // @ game+0x2d08ee0
	struct UAnimInstance* GetAnimInstance(); // Function Engine.SkeletalMeshComponent.GetAnimInstance // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d08ad0
	char GetAnimationMode(); // Function Engine.SkeletalMeshComponent.GetAnimationMode // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d08b10
	void ForceClothNextUpdateTeleportAndReset(); // Function Engine.SkeletalMeshComponent.ForceClothNextUpdateTeleportAndReset // Final|Native|Public|BlueprintCallable // @ game+0x2d085a0
	void ForceClothNextUpdateTeleport(); // Function Engine.SkeletalMeshComponent.ForceClothNextUpdateTeleport // Final|Native|Public|BlueprintCallable // @ game+0x2d08580
	void ClearMorphTargets(); // Function Engine.SkeletalMeshComponent.ClearMorphTargets // Final|Native|Public|BlueprintCallable // @ game+0x2d06140
	void BindClothToMasterPoseComponent(); // Function Engine.SkeletalMeshComponent.BindClothToMasterPoseComponent // Final|Native|Public|BlueprintCallable // @ game+0x2d05c80
	void AccumulateAllBodiesBelowPhysicsBlendWeight(struct FName InBoneName, float AddPhysicsBlendWeight, bool bSkipCustomPhysicsType); // Function Engine.SkeletalMeshComponent.AccumulateAllBodiesBelowPhysicsBlendWeight // Final|Native|Public|HasOutParms|BlueprintCallable // @ game+0x2d04280
};


// Class Engine.Material
// Size: 0xa18 (Inherited: 0x58)
struct UMaterial : UMaterialInterface {
	struct UPhysicalMaterial* PhysMaterial; // 0x58(0x08)
	struct FColorMaterialInput DiffuseColor; // 0x60(0x40)
	struct FColorMaterialInput SpecularColor; // 0xa0(0x40)
	struct FColorMaterialInput BaseColor; // 0xe0(0x40)
	struct FScalarMaterialInput Metallic; // 0x120(0x40)
	struct FScalarMaterialInput Specular; // 0x160(0x40)
	struct FScalarMaterialInput Roughness; // 0x1a0(0x40)
	struct FVectorMaterialInput Normal; // 0x1e0(0x48)
	struct FColorMaterialInput EmissiveColor; // 0x228(0x40)
	struct FScalarMaterialInput Opacity; // 0x268(0x40)
	struct FScalarMaterialInput OpacityMask; // 0x2a8(0x40)
	char MaterialDomain; // 0x2e8(0x01)
	char BlendMode; // 0x2e9(0x01)
	char DecalBlendMode; // 0x2ea(0x01)
	bool bDecalNoEmissiveOrRoughness; // 0x2eb(0x01)
	bool bDecalEnableStencil; // 0x2ec(0x01)
	char MaterialDecalResponse; // 0x2ed(0x01)
	char ShadingModel; // 0x2ee(0x01)
	char UnknownData_2EF[0x1]; // 0x2ef(0x01)
	float OpacityMaskClipValue; // 0x2f0(0x04)
	float DepthWeightScale; // 0x2f4(0x04)
	float DepthWeightExponent; // 0x2f8(0x04)
	float DepthWeightBase; // 0x2fc(0x04)
	char bBlendedOITAfterTranslucency : 1; // 0x300(0x01)
	char bForceResolution : 1; // 0x300(0x01)
	char UnknownData_300_2 : 6; // 0x300(0x01)
	char UnknownData_301[0x3]; // 0x301(0x03)
	char ForcedResolution; // 0x304(0x01)
	char UnknownData_305[0x3]; // 0x305(0x03)
	float FirstSplitDistance; // 0x308(0x04)
	float SecondSplitDistance; // 0x30c(0x04)
	float FullResInFrontMaxDistance; // 0x310(0x04)
	char FullResInFront : 1; // 0x314(0x01)
	char PopPerParticle : 1; // 0x314(0x01)
	char UnknownData_314_2 : 6; // 0x314(0x01)
	char UnknownData_315[0x3]; // 0x315(0x03)
	struct FVectorMaterialInput WorldPositionOffset; // 0x318(0x48)
	struct FVectorMaterialInput WorldDisplacement; // 0x360(0x48)
	struct FScalarMaterialInput TessellationMultiplier; // 0x3a8(0x40)
	struct FColorMaterialInput SubsurfaceColor; // 0x3e8(0x40)
	struct FScalarMaterialInput ClearCoat; // 0x428(0x40)
	struct FScalarMaterialInput ClearCoatRoughness; // 0x468(0x40)
	struct FScalarMaterialInput AmbientOcclusion; // 0x4a8(0x40)
	struct FScalarMaterialInput Refraction; // 0x4e8(0x40)
	struct FColorMaterialInput BillboardColor; // 0x528(0x40)
	struct FVectorMaterialInput BillboardParams; // 0x568(0x48)
	struct FVector2MaterialInput CustomizedUVs[0x08]; // 0x5b0(0x240)
	struct FMaterialAttributesInput MaterialAttributes; // 0x7f0(0x38)
	char UnknownData_828[0x10]; // 0x828(0x10)
	struct FScalarMaterialInput PixelDepthOffset; // 0x838(0x40)
	char bTranslucentBeforeTranslucency : 1; // 0x878(0x01)
	char bTranslucentBeforeVolumeFog : 1; // 0x878(0x01)
	char bTranslucentWithDepthWrite : 1; // 0x878(0x01)
	char bEnableSeparateTranslucency : 1; // 0x878(0x01)
	char bEnableResponsiveAA : 1; // 0x878(0x01)
	char bScreenSpaceReflections : 1; // 0x878(0x01)
	char TwoSided : 1; // 0x878(0x01)
	char DitheredLODTransition : 1; // 0x878(0x01)
	char UnknownData_879[0x3]; // 0x879(0x03)
	char ConstantMaterial; // 0x87c(0x01)
	char UnknownData_87D[0x3]; // 0x87d(0x03)
	float SkyLightWhenOccluded; // 0x880(0x04)
	float CloudShadowsInfluence; // 0x884(0x04)
	int32_t NumCustomizedUVs; // 0x888(0x04)
	char TranslucencyLightingMode; // 0x88c(0x01)
	char UnknownData_88D[0x3]; // 0x88d(0x03)
	float TranslucencyDirectionalLightingIntensity; // 0x890(0x04)
	float LightColorInfluence; // 0x894(0x04)
	float AmbientLightColorInfluence; // 0x898(0x04)
	float LightIntensityInfluence; // 0x89c(0x04)
	float AmbientLightIntensityInfluence; // 0x8a0(0x04)
	float AlternativeAmbientLightColorInfluence; // 0x8a4(0x04)
	float AlternativeAmbientLightIntensityInfluence; // 0x8a8(0x04)
	char bEvaluateLightningAtParticleCenter : 1; // 0x8ac(0x01)
	char UnknownData_8AC_1 : 7; // 0x8ac(0x01)
	char UnknownData_8AD[0x3]; // 0x8ad(0x03)
	float TranslucencyScaleForReflection; // 0x8b0(0x04)
	char bObjectFadeEnabled : 1; // 0x8b4(0x01)
	char UnknownData_8B4_1 : 7; // 0x8b4(0x01)
	char UnknownData_8B5[0x3]; // 0x8b5(0x03)
	float ObjectFadeRate; // 0x8b8(0x04)
	float ObjectFadeDistanceThreshold; // 0x8bc(0x04)
	float TranslucentShadowDensityScale; // 0x8c0(0x04)
	float TranslucentSelfShadowDensityScale; // 0x8c4(0x04)
	float TranslucentSelfShadowSecondDensityScale; // 0x8c8(0x04)
	float TranslucentSelfShadowSecondOpacity; // 0x8cc(0x04)
	float TranslucentBackscatteringExponent; // 0x8d0(0x04)
	struct FLinearColor TranslucentMultipleScatteringExtinction; // 0x8d4(0x10)
	float TranslucentShadowStartOffset; // 0x8e4(0x04)
	float VolumeFogScale; // 0x8e8(0x04)
	char DistanceFog : 1; // 0x8ec(0x01)
	char bDisableDepthTest : 1; // 0x8ec(0x01)
	char bGenerateSphericalParticleNormals : 1; // 0x8ec(0x01)
	char bDontRenderWhenPlayerInsideShip : 1; // 0x8ec(0x01)
	char bTangentSpaceNormal : 1; // 0x8ec(0x01)
	char bUseEmissiveForDynamicAreaLighting : 1; // 0x8ec(0x01)
	char bBlockGI : 1; // 0x8ec(0x01)
	char bUsedAsSpecialEngineMaterial : 1; // 0x8ec(0x01)
	char bUsedWithSkeletalMesh : 1; // 0x8ed(0x01)
	char bUsedWithEditorCompositing : 1; // 0x8ed(0x01)
	char bUsedWithLandscape : 1; // 0x8ed(0x01)
	char bUsedWithParticleSprites : 1; // 0x8ed(0x01)
	char bUsedWithCPUParticleSprites : 1; // 0x8ed(0x01)
	char bUsedWithStaticMesh : 1; // 0x8ed(0x01)
	char bUsedWithCoverage : 1; // 0x8ed(0x01)
	char bUsedWithGPUInstanceCulling : 1; // 0x8ed(0x01)
	char bUsedWithBeamTrails : 1; // 0x8ee(0x01)
	char bUsedWithMeshParticles : 1; // 0x8ee(0x01)
	char bUsedWithStaticLighting : 1; // 0x8ee(0x01)
	char bUsedWithFluidSurfaces : 1; // 0x8ee(0x01)
	char bUsedWithMorphTargets : 1; // 0x8ee(0x01)
	char bUsedWithSplineMeshes : 1; // 0x8ee(0x01)
	char bUsedWithInstancedStaticMeshes : 1; // 0x8ee(0x01)
	char bUsesDistortion : 1; // 0x8ee(0x01)
	char bUsedWithClothing : 1; // 0x8ef(0x01)
	char bUsedWithDeformables : 1; // 0x8ef(0x01)
	char bUsedWithFFTWaterMask : 1; // 0x8ef(0x01)
	char bUsedWithVolumeFogMask : 1; // 0x8ef(0x01)
	char bUsedWithDynamicObjectFading : 1; // 0x8ef(0x01)
	char bUsedWithWaterBoundary : 1; // 0x8ef(0x01)
	char bUsedWithClouds : 1; // 0x8ef(0x01)
	char bUsedWithInstancedRopes : 1; // 0x8ef(0x01)
	char bUsedWithPlanarReflection : 1; // 0x8f0(0x01)
	char bUsedWithPointLightShadows : 1; // 0x8f0(0x01)
	char bUsedWithMaskPass : 1; // 0x8f0(0x01)
	char bUsedWithUI : 1; // 0x8f0(0x01)
	char bAutomaticallySetUsageInEditor : 1; // 0x8f0(0x01)
	char bAllowEmissiveOnLandscape : 1; // 0x8f0(0x01)
	char bApplyWaterMask : 1; // 0x8f0(0x01)
	char bUseReZ : 1; // 0x8f0(0x01)
	char bFullyRough : 1; // 0x8f1(0x01)
	char bUseLightmapDirectionality : 1; // 0x8f1(0x01)
	char UnknownData_8F1_2 : 6; // 0x8f1(0x01)
	char UnknownData_8F2[0x2]; // 0x8f2(0x02)
	char GeometryMode; // 0x8f4(0x01)
	char D3D11TessellationMode; // 0x8f5(0x01)
	char UnknownData_8F6[0x2]; // 0x8f6(0x02)
	char bEnableCrackFreeDisplacement : 1; // 0x8f8(0x01)
	char bEnableAdaptiveTessellation : 1; // 0x8f8(0x01)
	char UnknownData_8F8_2 : 6; // 0x8f8(0x01)
	char UnknownData_8F9[0x3]; // 0x8f9(0x03)
	float MaxDisplacement; // 0x8fc(0x04)
	char Wireframe : 1; // 0x900(0x01)
	char bOutputVelocityOnBasePass : 1; // 0x900(0x01)
	char UnknownData_900_2 : 6; // 0x900(0x01)
	char UnknownData_901[0x3]; // 0x901(0x03)
	float NormalOffsetShadowBiasScale; // 0x904(0x04)
	int32_t EditorX; // 0x908(0x04)
	int32_t EditorY; // 0x90c(0x04)
	int32_t EditorPitch; // 0x910(0x04)
	int32_t EditorYaw; // 0x914(0x04)
	struct TArray<struct UMaterialExpression*> Expressions; // 0x918(0x10)
	struct TArray<struct FMaterialFunctionInfo> MaterialFunctionInfos; // 0x928(0x10)
	struct TArray<struct FMaterialParameterCollectionInfo> MaterialParameterCollectionInfos; // 0x938(0x10)
	char bCanMaskedBeAssumedOpaque : 1; // 0x948(0x01)
	char bIsMasked : 1; // 0x948(0x01)
	char bIsPreviewMaterial : 1; // 0x948(0x01)
	char bUseMaterialAttributes : 1; // 0x948(0x01)
	char bUseTranslucencyVertexFog : 1; // 0x948(0x01)
	char bUseDensityMapOnTranslucencyVertexFog : 1; // 0x948(0x01)
	char bAllowDevelopmentShaderCompile : 1; // 0x948(0x01)
	char bIsMaterialEditorStatsMaterial : 1; // 0x948(0x01)
	char UnknownData_949[0x3]; // 0x949(0x03)
	uint32_t UsageFlagWarnings; // 0x94c(0x04)
	char BlendableLocation; // 0x950(0x01)
	char UnknownData_951[0x3]; // 0x951(0x03)
	int32_t BlendablePriority; // 0x954(0x04)
	char bIsAlphaBlended : 1; // 0x958(0x01)
	char UnknownData_958_1 : 7; // 0x958(0x01)
	char UnknownData_959[0x3]; // 0x959(0x03)
	float RefractionDepthBias; // 0x95c(0x04)
	struct FGuid StateId; // 0x960(0x10)
	int32_t UnwrappedUVSet; // 0x970(0x04)
	char UnknownData_974[0x8c]; // 0x974(0x8c)
	struct TArray<struct UTexture*> ExpressionTextureReferences; // 0xa00(0x10)
	char UnknownData_A10[0x8]; // 0xa10(0x08)
};
// ScriptStruct PirateGenerator.RadialCoordinate
// Size: 0x08 (Inherited: 0x00)
struct FRadialCoordinate {
	float NormalizedAngle; // 0x00(0x04)
	float RadialDistance; // 0x04(0x04)
};

// ScriptStruct Athena.CarouselPirateDesc
// Size: 0x30 (Inherited: 0x00)
struct FCarouselPirateDesc {
	int32_t Seed; // 0x00(0x04)
	char Gender; // 0x04(0x01)
	char Ethnicity; // 0x05(0x01)
	char UnknownData_6[0x2]; // 0x06(0x02)
	float Age; // 0x08(0x04)
	struct FRadialCoordinate BodyShape; // 0x0c(0x08)
	float Dirtiness; // 0x14(0x04)
	float Wonkiness; // 0x18(0x04)
	char UnknownData_1C[0x4]; // 0x1c(0x04)
	struct TArray<struct UClass*> Outfit; // 0x20(0x10)
};

// Size: 0x450 (Inherited: 0x3c8)
struct PirateGeneratorLineUpUI : AActor {
	char pad1[0x3e8]; // 0x0 (0x3e8)
	struct TArray<struct FCarouselPirateDesc> CarouselPirateDescs; // 0x3e8(0x10)
};

// ScriptStruct CoreUObject.StringAssetReference
// Size: 0x10 (Inherited: 0x00)
struct FStringAssetReference {
	struct FString AssetLongPathname; // 0x00(0x10)
};

// ScriptStruct Athena.DetailAppearenceDesc
// Size: 0x30 (Inherited: 0x00)
struct FDetailAppearenceDesc {
	struct FStringAssetReference Mesh; // 0x00(0x10)
	struct TArray<struct FDetailAppearenceMaterialDesc> Materials; // 0x10(0x10)
	struct FStringAssetReference CustomDepthMaterial; // 0x20(0x10)
};
// Class Athena.WieldableItem
// Size: 0x7a0 (Inherited: 0x3c8)
struct AWieldableItem : AActor {
	char UnknownData_3C8[0x60]; // 0x3c8(0x60)
	struct UPrimitiveComponent* FirstPersonMesh; // 0x428(0x08)
	struct UPrimitiveComponent* ThirdPersonMesh; // 0x430(0x08)
	struct UActionRulesComponent* ActionRulesComponent; // 0x438(0x08)
	struct UClass* AnimationKey; // 0x440(0x08)
	struct FObjectMessagingDispatcher MessagingDispatcher; // 0x448(0xa0)
	char UnknownData_4E8[0x8]; // 0x4e8(0x08)
	struct FTransform AnimationOffset; // 0x4f0(0x30)
	struct UWwiseEvent* EquipSfx; // 0x520(0x08)
	struct UWwiseEvent* UnequipSfx; // 0x528(0x08)
	struct UWwiseEvent* WielderLandedJumpSfx; // 0x530(0x08)
	struct UWwiseEvent* WielderFootstepLayerOneshot; // 0x538(0x08)
	struct FName PlayerWalkSpeedRtpc; // 0x540(0x08)
	struct UWwiseEvent* FirstTimeEquipSfx; // 0x548(0x08)
	bool ForceUnequipSfx; // 0x550(0x01)
	char ItemSize; // 0x551(0x01)
	char UnknownData_552[0x6]; // 0x552(0x06)
	struct FDetailAppearenceDesc DetailAppearence; // 0x558(0x30)
	bool PropagateVisibilityToChildren; // 0x588(0x01)
	char UnknownData_589[0x3]; // 0x589(0x03)
	struct FName AINoiseTag; // 0x58c(0x08)
	char UnknownData_594[0x4]; // 0x594(0x04)
	struct TArray<struct FName> ThirdPersonExtraMeshNames; // 0x598(0x10)
	struct TArray<struct FName> StashedExtraMeshNames; // 0x5a8(0x10)
	struct UMeshComponent* StashedMeshComponent; // 0x5b8(0x08)
	struct FName StashedMeshAttachSocket; // 0x5c0(0x08)
	struct FName ServerWieldAttachSocket; // 0x5c8(0x08)
	struct UPhrasesRadialDataAsset* Phrases; // 0x5d0(0x08)
	struct UObject* CachedFirstPersonProxyMesh; // 0x5d8(0x08)
	struct UObject* CachedThirdPersonProxyMesh; // 0x5e0(0x08)
	char UnknownData_5E8[0x128]; // 0x5e8(0x128)
	struct UClass* ItemCategory; // 0x710(0x08)
	struct AItemInfo* ItemInfo; // 0x718(0x08)
	struct AInterpolatingItem* InterpolatingItem; // 0x720(0x08)
	char UnknownData_728[0x18]; // 0x728(0x18)
	struct AActor* LastKnownWielder; // 0x740(0x08)
	struct UWieldableItemVfxComponent* WieldableItemVfxComponent; // 0x748(0x08)
	struct TArray<struct UMeshComponent*> ThirdPersonExtraMeshes; // 0x750(0x10)
	struct TArray<struct UMeshComponent*> StashedExtraMeshes; // 0x760(0x10)
	char UnknownData_770[0x19]; // 0x770(0x19)
	bool OfferingEnabled; // 0x789(0x01)
	char UnknownData_78A[0x2]; // 0x78a(0x02)
	struct FGuid QuestId; // 0x78c(0x10)
	char UnknownData_79C[0x4]; // 0x79c(0x04)

	void SetItemWwiseSwitch(struct FName SwitchGroup, struct FName SwitchOption); // Function Athena.WieldableItem.SetItemWwiseSwitch // Final|Native|Public|HasOutParms|BlueprintCallable // @ game+0x3767b30
	void SetItemWwiseRTPC(struct FName RTPCName, float Value); // Function Athena.WieldableItem.SetItemWwiseRTPC // Final|Native|Public|HasOutParms|BlueprintCallable // @ game+0x3767a60
	void PostItemWwiseEvent(struct UWwiseEvent* Event); // Function Athena.WieldableItem.PostItemWwiseEvent // Final|Native|Public|BlueprintCallable // @ game+0x3767000
	void OnWielderHasLandedFromJumpBPImpl(); // Function Athena.WieldableItem.OnWielderHasLandedFromJumpBPImpl // Event|Public|BlueprintEvent // @ game+0x17bcfb0
	void OnRep_QuestId(); // Function Athena.WieldableItem.OnRep_QuestId // Final|Native|Protected // @ game+0x3766cb0
	void OnRep_ItemInfo(); // Function Athena.WieldableItem.OnRep_ItemInfo // Native|Protected // @ game+0x3766c90
	bool IsUnderwater(); // Function Athena.WieldableItem.IsUnderwater // Native|Protected|BlueprintCallable|BlueprintPure|Const // @ game+0x37664b0
	struct AActor* GetWielder(); // Function Athena.WieldableItem.GetWielder // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x37662a0
	struct FTransform GetWieldedItemAnimationOffset(); // Function Athena.WieldableItem.GetWieldedItemAnimationOffset // Final|Native|Public|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x3766150
	char GetWieldableItemSize(); // Function Athena.WieldableItem.GetWieldableItemSize // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x37660d0
	struct AActor* GetLastKnownWielder(); // Function Athena.WieldableItem.GetLastKnownWielder // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x3765d60
	bool GetIsFirstPerson(); // Function Athena.WieldableItem.GetIsFirstPerson // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x3765760
};

// Class Athena.InteractableBase
// Size: 0x400 (Inherited: 0x3c8)
struct AInteractableBase : AActor {
	char UnknownData_3C8[0x10]; // 0x3c8(0x10)
	bool RequiresFacingFront; // 0x3d8(0x01)
	bool RequiresNotBeingAirborne; // 0x3d9(0x01)
	bool RequiresNotSwimming; // 0x3da(0x01)
	bool InteractionsCanBeDisabled; // 0x3db(0x01)
	bool CanSetInteractionState; // 0x3dc(0x01)
	char UnknownData_3DD[0x3]; // 0x3dd(0x03)
	struct TArray<struct UInteractionPrerequisiteBase*> InteractionPrerequisites; // 0x3e0(0x10)
	struct UActionRulesComponent* ActionRulesComponent; // 0x3f0(0x08)
	char InteractableIdentifier; // 0x3f8(0x01)
	char UnknownData_3F9[0x1]; // 0x3f9(0x01)
	char CurrentInteractionState; // 0x3fa(0x01)
	char UnknownData_3FB[0x5]; // 0x3fb(0x05)
};
// Class Athena.InteractableObject
// Size: 0x468 (Inherited: 0x400)
struct AInteractableObject : AInteractableBase {
	char UnknownData_400[0x18]; // 0x400(0x18)
	struct UOneShotInteractableFilterDesc* FilterDesc; // 0x418(0x08)
	struct FText Verb; // 0x420(0x38)
	char UnknownData_458[0x10]; // 0x458(0x10)

	void ServerInteract(struct AActor* InInteractor); // Function Athena.InteractableObject.ServerInteract // Final|Native|Public|BlueprintCallable // @ game+0x36cab80
	void Blueprint_OnInteract_Server(struct AActor* InInteractor); // Function Athena.InteractableObject.Blueprint_OnInteract_Server // Event|Protected|BlueprintEvent // @ game+0x17bcfb0
	void Blueprint_OnInteract_Client(struct AActor* InInteractor); // Function Athena.InteractableObject.Blueprint_OnInteract_Client // Event|Protected|BlueprintEvent // @ game+0x17bcfb0
};
// Class Engine.StaticMeshComponent
// Size: 0x620 (Inherited: 0x5d0)
struct UStaticMeshComponent : UMeshComponent {
	int32_t ForcedLodModel; // 0x5c8(0x04)
	bool bOverrideMinLOD; // 0x5cc(0x01)
	bool bOverrideNavigationToDefaultObstacle; // 0x5cd(0x01)
	int32_t MinLOD; // 0x5d0(0x04)
	struct UStaticMesh* StaticMesh; // 0x5d8(0x08)
	bool bOverrideWireframeColor; // 0x5e0(0x01)
	char UnknownData_5E3[0x1]; // 0x5e3(0x01)
	struct FColor WireframeColorOverride; // 0x5e4(0x04)
	char bIgnoreInstanceForTextureStreaming : 1; // 0x5e8(0x01)
	char bOverrideLightMapRes : 1; // 0x5e8(0x01)
	char UnknownData_5E8_2 : 6; // 0x5e8(0x01)
	char UnknownData_5E9[0x3]; // 0x5e9(0x03)
	int32_t OverriddenLightMapRes; // 0x5ec(0x04)
	float StreamingDistanceMultiplier; // 0x5f0(0x04)
	char UnknownData_5F4[0x4]; // 0x5f4(0x04)
	struct TArray<struct FGuid> IrrelevantLights; // 0x5f8(0x10)
	struct TArray<struct FStaticMeshComponentLODInfo> LODData; // 0x608(0x10)
	struct UMaterialInterface* CustomDepthMaterial; // 0x618(0x08)

	bool SetStaticMesh(struct UStaticMesh* NewMesh); // Function Engine.StaticMeshComponent.SetStaticMesh // Native|Public|BlueprintCallable // @ game+0x2e22650
	void SetForcedLodModel(int32_t NewForcedLodModel); // Function Engine.StaticMeshComponent.SetForcedLodModel // Final|Native|Public|BlueprintCallable // @ game+0x2e1f940
	void SetCustomDepthMaterial(struct UMaterialInterface* Material); // Function Engine.StaticMeshComponent.SetCustomDepthMaterial // Final|Native|Public|BlueprintCallable // @ game+0x2e1e530
	void OnRep_StaticMesh(struct UStaticMesh* OldStaticMesh); // Function Engine.StaticMeshComponent.OnRep_StaticMesh // Final|Native|Public // @ game+0x2e19d50
	void GetLocalBounds(struct FVector Min, struct FVector Max); // Function Engine.StaticMeshComponent.GetLocalBounds // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2e16810
	struct UMaterialInterface* GetCustomDepthMaterial(); // Function Engine.StaticMeshComponent.GetCustomDepthMaterial // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2e15550
};
// Class Athena.StorageContainer
// Size: 0x4c8 (Inherited: 0x468)
struct AStorageContainer : AInteractableObject {
	char UnknownData_468[0x8]; // 0x468(0x08)
	struct UStaticMeshComponent* Mesh; // 0x470(0x08)
	struct UInteractableComponent* InteractionRegion; // 0x478(0x08)
	char UnknownData_480[0x14]; // 0x480(0x14)
	char TrackedActorType; // 0x494(0x01)
	char UnknownData_495[0x33]; // 0x495(0x33)
};

// Class Athena.Sail
// Size: 0x570 (Inherited: 0x3c8)
struct ASail : AActor {
	char UnknownData_3C8[0x10]; // 0x3c8(0x10)
	struct USkeletalMeshComponent* Mesh; // 0x3d8(0x08)
	struct TArray<struct UMaterialInstanceDynamic*> DynamicMaterials; // 0x3e0(0x10)
	struct FVector PositionForSailDynamicsRelativeToSailActor; // 0x3f0(0x0c)
	char UnknownData_3FC[0x4]; // 0x3fc(0x04)
	struct UBaseSailDynamicsDesc* SailDynamicsDesc; // 0x400(0x08)
	float MinRelativeWindSpdForFullVisibleInflation; // 0x408(0x04)
	float BaseWindStrength; // 0x40c(0x04)
	struct UCurveFloat* WindToSailAngleEfficiencyCurve; // 0x410(0x08)
	float NormalBillowLimit; // 0x418(0x04)
	float PerfectBillowValue; // 0x41c(0x04)
	float PerfectBillowThreshold; // 0x420(0x04)
	float SailLoweredPerfectWindThreshold; // 0x424(0x04)
	float BlendSpeed; // 0x428(0x04)
	struct FName FurlMaterialParameterName; // 0x42c(0x08)
	float AnimationSailHeightScalar; // 0x434(0x04)
	char UnknownData_438[0x104]; // 0x438(0x104)
	float DamageScalar; // 0x53c(0x04)
	char UnknownData_540[0x30]; // 0x540(0x30)
};

// Class Athena.DrunkennessComponent
// Size: 0x258 (Inherited: 0xc8)
struct UDrunkennessComponent : UActorComponent{
	char UnknownData_C8[0x8]; // 0xc8(0x08)
	struct UDrunkennessComponentPublicData* PublicData; // 0xd0(0x08)
	char UnknownData_D8[0x140]; // 0xd8(0x140)
	float TargetDrunkenness[0x02]; // 0x218(0x08)
	float CurrentDrunkenness[0x02]; // 0x220(0x08)
	float RemainingAmountToSoberUpDueToWaterSplash; // 0x228(0x04)
	char UnknownData_22C[0xc]; // 0x22c(0x0c)

	void TestForceVomiting(float Duration); // Function Athena.DrunkennessComponent.TestForceVomiting // Final|Native|Public|BlueprintCallable // @ game+0xe8f1f0
	void SetTargetDrunkenness(char Type, float InTargetValue); // Function Athena.DrunkennessComponent.SetTargetDrunkenness // Final|Native|Public|BlueprintCallable // @ game+0xe8ea90
	void Reset(); // Function Athena.DrunkennessComponent.Reset // Final|Native|Public|BlueprintCallable // @ game+0xe8d360
	void OnRep_TargetDrunkenness(); // Function Athena.DrunkennessComponent.OnRep_TargetDrunkenness // Final|Native|Private // @ game+0xe8c180
	void OnRep_RemainingAmountToSoberUpDueToWaterSplash(); // Function Athena.DrunkennessComponent.OnRep_RemainingAmountToSoberUpDueToWaterSplash // Final|Native|Private // @ game+0xe8bf80
	void OnRep_CurrentDrunkenness(); // Function Athena.DrunkennessComponent.OnRep_CurrentDrunkenness // Final|Native|Private // @ game+0xe8bd70
	float GetStaggerAmplitude(); // Function Athena.DrunkennessComponent.GetStaggerAmplitude // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xe8a4c0
	float GetOverallCurrentDrunkenness(); // Function Athena.DrunkennessComponent.GetOverallCurrentDrunkenness // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xe89e00
	void ChangeDrunkenness(char Type, float DrunkennessChange) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.DrunkennessComponent.ChangeDrunkenness");
		struct {
			char T = 0;
			float Dchange = 0.f;
		}params;
		Type = params.T;
		DrunkennessChange = params.Dchange;
		ProcessEvent(this, fn, &params);
	}; // Function Athena.DrunkennessComponent.ChangeDrunkenness // Final|Native|Public|BlueprintCallable // @ game+0xe87b10
};
// ScriptStruct Athena.DrunkennessSetupData
// Size: 0x14 (Inherited: 0x00)
struct FDrunkennessSetupData {
	char DrunkennessType; // 0x00(0x01)
	char UnknownData_1[0x3]; // 0x01(0x03)
	float MaxDrunkenness; // 0x04(0x04)
	float GettingDrunkDrunkennessChangePerSec; // 0x08(0x04)
	float SoberingUpDrunkennessChangePerSec; // 0x0c(0x04)
	bool AutoSoberUpWhenFullyDrunk; // 0x10(0x01)
	char UnknownData_11[0x3]; // 0x11(0x03)
};

// Class Athena.DrunkennessComponentPublicData
// Size: 0xb0 (Inherited: 0x28)
struct UDrunkennessComponentPublicData {
	struct TArray<struct FDrunkennessSetupData> DrunkennessSetupData; // 0x28(0x10)
	float VomitingThresholdWhenGettingDrunker; // 0x38(0x04)
	float VomitingThresholdWhenSobering; // 0x3c(0x04)
	float MinTimeBetweenVomitSpews; // 0x40(0x04)
	float MaxTimeBetweenVomitSpews; // 0x44(0x04)
	float MinVomitSpewDuration; // 0x48(0x04)
	float MaxVomitSpewDuration; // 0x4c(0x04)
	float WaterSplashSoberingAmount; // 0x50(0x04)
	float WaterSplashSoberingRate; // 0x54(0x04)
	struct UCurveFloat* DrunkennessRemappingCurveForScreenVfx; // 0x58(0x08)
	struct UCurveFloat* DrunkennessRemappingCurveForStaggering; // 0x60(0x08)
	float CameraRollAmp; // 0x68(0x04)
	float CameraRollAccel; // 0x6c(0x04)
	struct UWwiseObjectPoolWrapper* DrunkennessComponentPool; // 0x70(0x08)
	struct UWwiseEvent* StartDrunkennessSfx; // 0x78(0x08)
	struct UWwiseEvent* StopDrunkennessSfx; // 0x80(0x08)
	struct UWwiseEvent* StartDrunkennessSfxRemotePlayer; // 0x88(0x08)
	struct UWwiseEvent* StopDrunkennessSfxRemotePlayer; // 0x90(0x08)
	struct FName PlayerDrunkennessAmountRtpc; // 0x98(0x08)
	struct FName RemotePlayerDrunkennessAmountRtpc; // 0xa0(0x08)
	float MinDrunkennessToToggleLocomotionAnimType; // 0xa8(0x04)
	char UnknownData_AC[0x4]; // 0xac(0x04)
};

// ScriptStruct Athena.LightingControllerSettingsOutput
// Size: 0x378 (Inherited: 0x00)
struct FLightingControllerSettingsOutput {
	float FogDensity; // 0x00(0x04)
	float FogStartDistance; // 0x04(0x04)
	float LightIntensity; // 0x08(0x04)
	float SkyDomeBrightness; // 0x0c(0x04)
	float SkyDomeMoonVisibility; // 0x10(0x04)
	float SkyDomeStarBrightness; // 0x14(0x04)
	float SkyDomeSunBrightness; // 0x18(0x04)
	float PostLPVIntensity; // 0x1c(0x04)
	float PostExposureBias; // 0x20(0x04)
	float PostLPVOcclusionIntensity; // 0x24(0x04)
	float SkyLightIntensity; // 0x28(0x04)
	float FogBrightnessNear; // 0x2c(0x04)
	float FogBrightnessFar; // 0x30(0x04)
	float PostBloomIntensity; // 0x34(0x04)
	float PostBloomThreshold; // 0x38(0x04)
	float SkyDomeAuraBrightness; // 0x3c(0x04)
	float PostReflectionProbeBrightness; // 0x40(0x04)
	float PostReflectionProbeShadowBrightness; // 0x44(0x04)
	float VfxController; // 0x48(0x04)
	float FogMaxOpacity; // 0x4c(0x04)
	float FogScatterExp; // 0x50(0x04)
	float FogUnderwaterDensity; // 0x54(0x04)
	float PostLensFlareIntensity; // 0x58(0x04)
	float EmissiveFoliage; // 0x5c(0x04)
	float EmissiveLights; // 0x60(0x04)
	float PostExposureMinBrightness; // 0x64(0x04)
	float PostExposureMaxBrightness; // 0x68(0x04)
	float SkyDomeMoonBrightness; // 0x6c(0x04)
	float SkyDomeHorizonCloudBrightness; // 0x70(0x04)
	float SkyDomeGreenFlashStrength; // 0x74(0x04)
	float PostSaturation; // 0x78(0x04)
	float PostContrast; // 0x7c(0x04)
	float CloudRimlightScale; // 0x80(0x04)
	float CloudSkylightScale; // 0x84(0x04)
	float CloudSunlightScale; // 0x88(0x04)
	float CloudGlobalPressure; // 0x8c(0x04)
	float PostUnderWaterBrightness; // 0x90(0x04)
	float VfxCloudBrightness; // 0x94(0x04)
	float LightGodRayIntensity; // 0x98(0x04)
	float CloudBottomBrightness; // 0x9c(0x04)
	float CloudSunlightShadowing; // 0xa0(0x04)
	float SunDiscScale; // 0xa4(0x04)
	float WaterSpecularBrightness; // 0xa8(0x04)
	float VfxAmbientScale; // 0xac(0x04)
	float FogUnderwaterMaxOpacity; // 0xb0(0x04)
	float FogUnderwaterScatterExp; // 0xb4(0x04)
	float FogUnderwaterStartDistance; // 0xb8(0x04)
	float FogUnderwaterBrightnessNear; // 0xbc(0x04)
	float FogUnderwaterBrightnessFar; // 0xc0(0x04)
	float FogDirectionalInscatteringStart; // 0xc4(0x04)
	float FogUnderwaterDirectionalInscatteringStart; // 0xc8(0x04)
	float CloudShadowBrightness; // 0xcc(0x04)
	float StormCloudShadowBrightness; // 0xd0(0x04)
	float StormPostBloomIntensity; // 0xd4(0x04)
	float StormPostBloomThreshold; // 0xd8(0x04)
	float DirectionalLightShadowSteppingRate; // 0xdc(0x04)
	float SkullBrightness; // 0xe0(0x04)
	float UnderwaterDirectionalLightMinBrightness; // 0xe4(0x04)
	struct FLinearColor FogColourAway; // 0xe8(0x10)
	struct FLinearColor FogColourTowards; // 0xf8(0x10)
	struct FLinearColor LightSunColour; // 0x108(0x10)
	struct FLinearColor SkyDomeHorizonColour; // 0x118(0x10)
	struct FLinearColor SkyDomeZenithColour; // 0x128(0x10)
	struct FLinearColor SkyLightColour; // 0x138(0x10)
	struct FLinearColor PostAmbientShadowColour; // 0x148(0x10)
	struct FLinearColor FogColourAwayFar; // 0x158(0x10)
	struct FLinearColor FogColourUnderwater; // 0x168(0x10)
	struct FLinearColor FogColourAwayUnderwater; // 0x178(0x10)
	struct FLinearColor WaterColour; // 0x188(0x10)
	struct FLinearColor WaterColourShallow; // 0x198(0x10)
	struct FLinearColor WaterBackLitColour; // 0x1a8(0x10)
	struct FLinearColor WaterFoamColour; // 0x1b8(0x10)
	struct FLinearColor WaterFoamHighlightColour; // 0x1c8(0x10)
	struct FLinearColor WaterAmbientColour; // 0x1d8(0x10)
	struct FLinearColor PostReflectionProbeTint; // 0x1e8(0x10)
	struct FLinearColor CloudRimlightColour; // 0x1f8(0x10)
	struct FLinearColor CloudSkylightColour; // 0x208(0x10)
	struct FLinearColor CloudSunlightColour; // 0x218(0x10)
	struct FLinearColor PostUnderWaterTint; // 0x228(0x10)
	struct FLinearColor SkyDomeMoonColour; // 0x238(0x10)
	struct FLinearColor PostGamma; // 0x248(0x10)
	struct FLinearColor SkyDomeSunColour; // 0x258(0x10)
	struct FLinearColor VFXFoamColour; // 0x268(0x10)
	struct FLinearColor VFXWindColour; // 0x278(0x10)
	struct FLinearColor VFXUnderwaterTint; // 0x288(0x10)
	struct FLinearColor StormRainColour; // 0x298(0x10)
	struct FLinearColor StormSkyLightColour; // 0x2a8(0x10)
	struct FLinearColor StormPostGamma; // 0x2b8(0x10)
	struct FLinearColor StormFogColourUnderwater; // 0x2c8(0x10)
	struct FLinearColor StormFogColourAwayUnderwater; // 0x2d8(0x10)
	struct FLinearColor StormWaterColour; // 0x2e8(0x10)
	struct FLinearColor StormWaterColourShallow; // 0x2f8(0x10)
	struct FLinearColor StormWaterBackLitColour; // 0x308(0x10)
	struct FLinearColor StormWaterFoamColour; // 0x318(0x10)
	struct FLinearColor StormWaterFoamHighlightColour; // 0x328(0x10)
	struct FLinearColor StormWaterAmbientColour; // 0x338(0x10)
	struct FLinearColor StormPostReflectionProbeTint; // 0x348(0x10)
	struct FLinearColor SkullEyeColour; // 0x358(0x10)
	struct FLinearColor SandColour; // 0x368(0x10)
};

// Class Engine.LightComponentBase
// Size: 0x310 (Inherited: 0x2e0)
struct ULightComponentBase : USceneComponent {
	struct FGuid LightGuid; // 0x2e0(0x10)
	float Brightness; // 0x2f0(0x04)
	float Intensity; // 0x2f4(0x04)
	struct FColor LightColor; // 0x2f8(0x04)
	char bAffectsWorld : 1; // 0x2fc(0x01)
	char CastShadows : 1; // 0x2fc(0x01)
	char CastStaticShadows : 1; // 0x2fc(0x01)
	char CastDynamicShadows : 1; // 0x2fc(0x01)
	char bAffectTranslucentLighting : 1; // 0x2fc(0x01)
	char bPrecomputedLightingIsValid : 1; // 0x2fc(0x01)
	char RainOcclusion : 1; // 0x2fc(0x01)
	char UnknownData_2FD[0x3]; // 0x2fd(0x03)
	float IndirectLightingIntensity; // 0x300(0x04)
	float ScaleForTranslucency; // 0x304(0x04)
	float CapForTranslucency; // 0x308(0x04)
	char UnknownData_30C[0x4]; // 0x30c(0x04)

	void SetCastShadows(bool bNewValue); // Function Engine.LightComponentBase.SetCastShadows // Final|Native|Public|BlueprintCallable // @ game+0x2d127c0
	struct FLinearColor GetLightColor(); // Function Engine.LightComponentBase.GetLightColor // Final|Native|Public|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2d099d0
};
// Class Engine.Light
// Size: 0x3d0 (Inherited: 0x3c8)
struct ALight {
	struct ULightComponent* LightComponent; // 0x3c8(0x08)

	void SetLightFunctionScale(struct FVector NewLightFunctionScale); // Function Engine.Light.SetLightFunctionScale // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d147c0
	void SetLightFunctionMaterial(struct UMaterialInterface* NewLightFunctionMaterial); // Function Engine.Light.SetLightFunctionMaterial // Final|Native|Public|BlueprintCallable // @ game+0x2d146c0
	void SetLightFunctionFadeDistance(float NewLightFunctionFadeDistance); // Function Engine.Light.SetLightFunctionFadeDistance // Final|Native|Public|BlueprintCallable // @ game+0x2d145c0
	void SetLightColor(struct FLinearColor NewLightColor)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Light.SetLightColor");
		FLinearColor params{};
		NewLightColor = params;
		ProcessEvent(this, fn, &params);
	}; // Function Engine.Light.SetLightColor // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d14370
	void SetCastShadows(bool bNewValue); // Function Engine.Light.SetCastShadows // Final|Native|Public|BlueprintCallable // @ game+0x2d12730
	void SetBrightness(float NewBrightness); // Function Engine.Light.SetBrightness // Final|Native|Public|BlueprintCallable // @ game+0x2d12590
	void SetAffectTranslucentLighting(bool bNewValue); // Function Engine.Light.SetAffectTranslucentLighting // Final|Native|Public|BlueprintCallable // @ game+0x2d10b10
	struct FLinearColor GetLightColor(); // Function Engine.Light.GetLightColor // Final|Native|Public|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2d09990
	float GetBrightness(); // Function Engine.Light.GetBrightness // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d08cd0
};

// Class Engine.PointLight
// Size: 0x3d8 (Inherited: 0x3d0)
struct APointLight : ALight {
	struct UPointLightComponent* PointLightComponent; // 0x3d0(0x08)

	void SetRadius(float NewRadius); // Function Engine.PointLight.SetRadius // Final|RequiredAPI|Native|Public|BlueprintCallable // @ game+0x2d164d0
	void SetLightFalloffExponent(float NewLightFalloffExponent); // Function Engine.PointLight.SetLightFalloffExponent // Final|RequiredAPI|Native|Public|BlueprintCallable // @ game+0x2d144c0
};

// Class Engine.LightComponent
// Size: 0x470 (Inherited: 0x310)
struct ULightComponent : ULightComponentBase {
	float Temperature; // 0x310(0x04)
	char bUseTemperature : 1; // 0x314(0x01)
	char UnknownData_314_1 : 7; // 0x314(0x01)
	char UnknownData_315[0x3]; // 0x315(0x03)
	int32_t ShadowMapChannel; // 0x318(0x04)
	char UnknownData_31C[0x4]; // 0x31c(0x04)
	float SourceRadius; // 0x320(0x04)
	float MinRoughness; // 0x324(0x04)
	float ShadowBias; // 0x328(0x04)
	float ShadowSharpen; // 0x32c(0x04)
	float ShadowResMaxDynamicDegradation; // 0x330(0x04)
	char InverseSquaredFalloff : 1; // 0x334(0x01)
	char CastTranslucentShadows : 1; // 0x334(0x01)
	char bCastShadowsFromCinematicObjectsOnly : 1; // 0x334(0x01)
	char bAffectDynamicIndirectLighting : 1; // 0x334(0x01)
	char UnknownData_334_4 : 4; // 0x334(0x01)
	char UnknownData_335[0x3]; // 0x335(0x03)
	struct UMaterialInterface* LightFunctionMaterial; // 0x338(0x08)
	struct FVector LightFunctionScale; // 0x340(0x0c)
	char UnknownData_34C[0x4]; // 0x34c(0x04)
	struct UTextureLightProfile* IESTexture; // 0x350(0x08)
	char bUseIESBrightness : 1; // 0x358(0x01)
	char UnknownData_358_1 : 7; // 0x358(0x01)
	char UnknownData_359[0x3]; // 0x359(0x03)
	float IESBrightnessScale; // 0x35c(0x04)
	float LightFunctionFadeDistance; // 0x360(0x04)
	float DisabledBrightness; // 0x364(0x04)
	char bEnableLightShaftBloom : 1; // 0x368(0x01)
	char UnknownData_368_1 : 7; // 0x368(0x01)
	char UnknownData_369[0x3]; // 0x369(0x03)
	float BloomScale; // 0x36c(0x04)
	float BloomThreshold; // 0x370(0x04)
	struct FColor BloomTint; // 0x374(0x04)
	bool bUseRayTracedDistanceFieldShadows; // 0x378(0x01)
	char UnknownData_379[0x3]; // 0x379(0x03)
	float RayStartOffsetDepthScale; // 0x37c(0x04)
	char UnknownData_380[0xf0]; // 0x380(0xf0)

	void SetTemperature(float NewTemperature); // Function Engine.LightComponent.SetTemperature // Final|Native|Public|BlueprintCallable // @ game+0x2d16e20
	void SetLightFunctionScale(struct FVector NewLightFunctionScale); // Function Engine.LightComponent.SetLightFunctionScale // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d14850
	void SetLightFunctionMaterial(struct UMaterialInterface* NewLightFunctionMaterial); // Function Engine.LightComponent.SetLightFunctionMaterial // Final|Native|Public|BlueprintCallable // @ game+0x2d14740
	void SetLightFunctionFadeDistance(float NewLightFunctionFadeDistance); // Function Engine.LightComponent.SetLightFunctionFadeDistance // Final|Native|Public|BlueprintCallable // @ game+0x2d14640
	void SetLightFunctionDisabledBrightness(float NewValue); // Function Engine.LightComponent.SetLightFunctionDisabledBrightness // Final|Native|Public|BlueprintCallable // @ game+0x2d14540
	void SetLightColor(struct FLinearColor NewLightColor, bool bSRGB)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.LightComponent.SetBloomTint");
		struct {
			FLinearColor params;
			bool yes = false;
		}params;
		NewLightColor = params.params;
		bSRGB = params.yes;
		ProcessEvent(this, fn, &params);
	}; // Function Engine.LightComponent.SetLightColor // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d143f0
	void SetIntensity(float NewIntensity); // Function Engine.LightComponent.SetIntensity // Final|Native|Public|BlueprintCallable // @ game+0x2d142f0
	void SetIndirectLightingIntensity(float NewIntensity); // Function Engine.LightComponent.SetIndirectLightingIntensity // Final|Native|Public|BlueprintCallable // @ game+0x2d141f0
	void SetIESTexture(struct UTextureLightProfile* NewValue); // Function Engine.LightComponent.SetIESTexture // Final|Native|Public|BlueprintCallable // @ game+0x2d14170
	void SetEnableLightShaftBloom(bool bNewValue); // Function Engine.LightComponent.SetEnableLightShaftBloom // Final|Native|Public|BlueprintCallable // @ game+0x2d13da0
	void SetBloomTint(struct FColor NewValue)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.LightComponent.SetBloomTint");
		FColor params{};
		NewValue = params;
		ProcessEvent(this, fn, &params);
	}; // Function Engine.LightComponent.SetBloomTint // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2d12520
	void SetBloomThreshold(float NewValue); // Function Engine.LightComponent.SetBloomThreshold // Final|Native|Public|BlueprintCallable // @ game+0x2d124a0
	void SetBloomScale(float NewValue); // Function Engine.LightComponent.SetBloomScale // Final|Native|Public|BlueprintCallable // @ game+0x2d12420
	void SetAffectTranslucentLighting(bool bNewValue); // Function Engine.LightComponent.SetAffectTranslucentLighting // Final|Native|Public|BlueprintCallable // @ game+0x2d10ba0
	void SetAffectDynamicIndirectLighting(bool bNewValue); // Function Engine.LightComponent.SetAffectDynamicIndirectLighting // Final|Native|Public|BlueprintCallable // @ game+0x2d10a80
	int32_t GetNumberOfShadowCastersInFirstCascade(); // Function Engine.LightComponent.GetNumberOfShadowCastersInFirstCascade // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2d0a610
};

struct ASkyLight{
	struct USkyLightComponent* LightComponent; // 0x3c8(0x08)
	char bEnabled : 1; // 0x3d0(0x01)
	char UnknownData_3D0_1 : 7; // 0x3d0(0x01)
	char UnknownData_3D1[0x7]; // 0x3d1(0x07)

	void OnRep_bEnabled(); // Function Engine.SkyLight.OnRep_bEnabled // Native|Public // @ game+0x2d0f750
};
// Class Engine.SkyLightComponent
// Size: 0x4b0 (Inherited: 0x310)
struct USkyLightComponent : ULightComponentBase {
	char SourceType; // 0x310(0x01)
	char UnknownData_311[0x7]; // 0x311(0x07)
	struct UTextureCube* Cubemap; // 0x318(0x08)
	float SkyDistanceThreshold; // 0x320(0x04)
	bool bLowerHemisphereIsBlack; // 0x324(0x01)
	char UnknownData_325[0x3]; // 0x325(0x03)
	float OcclusionMaxDistance; // 0x328(0x04)
	float Contrast; // 0x32c(0x04)
	float MinOcclusion; // 0x330(0x04)
	struct FColor OcclusionTint; // 0x334(0x04)
	float ScaleForInteriors; // 0x338(0x04)
	char UnknownData_33C[0xac]; // 0x33c(0xac)
	struct UTextureCube* BlendDestinationCubemap; // 0x3e8(0x08)
	char UnknownData_3F0[0xc0]; // 0x3f0(0xc0)

	void SetOcclusionTint(struct FColor InTint); // Function Engine.SkyLightComponent.SetOcclusionTint // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable // @ game+0x2e20e80
	void SetMinOcclusion(float InMinOcclusion); // Function Engine.SkyLightComponent.SetMinOcclusion // Final|Native|Public|BlueprintCallable // @ game+0x2e20880
	void SetLightColor(struct FLinearColor NewLightColor)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.SkyLightComponent.SetLightColor");
		FLinearColor params;
		NewLightColor = params;
		ProcessEvent(this, fn, &params);
	}; // Function Engine.SkyLightComponent.SetLightColor // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2e204d0
	void SetIntensity(float NewIntensity); // Function Engine.SkyLightComponent.SetIntensity // Final|Native|Public|BlueprintCallable // @ game+0x2e201d0
	void SetIndirectLightingIntensity(float NewIntensity); // Function Engine.SkyLightComponent.SetIndirectLightingIntensity // Final|Native|Public|BlueprintCallable // @ game+0x2e1ff10
	void SetCubemapBlend(struct UTextureCube* SourceCubemap, struct UTextureCube* DestinationCubemap, float InBlendFraction); // Function Engine.SkyLightComponent.SetCubemapBlend // Final|Native|Public|BlueprintCallable // @ game+0x2e1e360
	void SetCubemap(struct UTextureCube* NewCubemap); // Function Engine.SkyLightComponent.SetCubemap // Final|Native|Public|BlueprintCallable // @ game+0x2e1e2e0
	void RecaptureSky(); // Function Engine.SkyLightComponent.RecaptureSky // Final|Native|Public|BlueprintCallable // @ game+0x2e1b580
};