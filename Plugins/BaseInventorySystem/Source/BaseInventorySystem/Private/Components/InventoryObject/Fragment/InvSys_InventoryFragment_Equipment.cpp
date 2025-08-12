// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_Equipment.h"

#include "BaseInventorySystem.h"
#include "NativeGameplayTags.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_ItemFragment_EquipItem.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"

UInvSys_InventoryFragment_Equipment::UInvSys_InventoryFragment_Equipment()
{
	Priority = 10;
}

void UInvSys_InventoryFragment_Equipment::RefreshInventoryFragment()
{
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("正在刷新装备片段[%s]"), *GetInventoryTag().ToString())
	if (EquipmentInstance)
	{
		BroadcastEquipItemInstance(EquipmentInstance);
	}
	else
	{
		BroadcastUnEquipItemInstance();
	}
}

void UInvSys_InventoryFragment_Equipment::InitInventoryFragment(UObject* PreEditFragment)
{
	Super::InitInventoryFragment(PreEditFragment);
	// COPY_INVENTORY_FRAGMENT_PROPERTY(UInvSys_InventoryFragment_Equipment, SupportEquipTag);
}

UInvSys_InventoryItemInstance* UInvSys_InventoryFragment_Equipment::EquipItemDefinition(
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef)
{
	check(HasAuthority())
	if (HasEquipmentItems())
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, Has equip other items."), __FUNCTION__)
		return nullptr;
	}
	if (ItemDef == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemDefinition is nullptr."), __FUNCTION__)
		return nullptr;
	}

	/**
	 * 物品的定义中必须包含装备片段
	 */
	auto CDO_ItemDefinition = ItemDef->GetDefaultObject<UInvSys_InventoryItemDefinition>();
	if (CDO_ItemDefinition == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, CDO_ItemDefinition is nullptr."), __FUNCTION__)
		return nullptr;
	}
	auto EquipmentFragment = CDO_ItemDefinition->FindFragmentByClass<UInvSys_ItemFragment_EquipItem>();
	if (EquipmentFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, EquipmentFragment is nullptr."), __FUNCTION__)
		return nullptr;
	}
	if (EquipmentFragment->SupportEquipSlot.HasTagExact(InventoryTag) == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, 物品支持的装备槽中不包括 %s ."), __FUNCTION__, *InventoryTag.ToString())
		return nullptr;
	}

	UInvSys_InventoryItemInstance* TempItemInstance = NewObject<UInvSys_InventoryItemInstance>(GetInventoryComponent());
	if (TempItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr, %s is not valid."), __FUNCTION__, *ItemDef->GetName())
		return nullptr;
	}
	EquipmentInstance = TempItemInstance;
	EquipmentInstance->SetItemDefinition(ItemDef);
	EquipmentInstance->SetItemUniqueID(FGuid::NewGuid());
	EquipmentInstance->SetSlotTag(GetInventoryTag());
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRep_ItemInstance();
	}
	return EquipmentInstance;
}

UInvSys_InventoryItemInstance* UInvSys_InventoryFragment_Equipment::EquipItemInstance(UInvSys_InventoryItemInstance* ItemInstance)
{
	check(HasAuthority())
	if (HasEquipmentItems())
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, Has equip other items."), __FUNCTION__)
		return nullptr;
	}
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return nullptr;
	}

	/**
	 * 物品的定义中必须包含装备片段
	 */
	auto EquipItemFragment = ItemInstance->FindFragmentByClass<UInvSys_ItemFragment_EquipItem>();
	if (EquipItemFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, EquipmentFragment is nullptr."), __FUNCTION__)
		return nullptr;
	}
	if (EquipItemFragment->SupportEquipSlot.HasTagExact(InventoryTag) == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, 物品支持的装备槽中不包括 %s ."), __FUNCTION__, *InventoryTag.ToString())
		return nullptr;
	}

	/**
	 * 更新物品的库存组件
	 * todo::直接使用 SetInventoryComponent 是否会导致客户端同步失效？
	 */
	UInvSys_InventoryItemInstance* TargetItemInstance = ItemInstance;
	if (ItemInstance->GetInventoryComponent() != GetInventoryComponent())
	{
		TargetItemInstance = DuplicateObject(ItemInstance, GetInventoryComponent());
		ItemInstance->ConditionalBeginDestroy(); // 通知 GC 清理对象
	}
	if (TargetItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, Duplicate ItemInstance is nullptr."), __FUNCTION__)
		return nullptr;
	}

	EquipmentInstance = TargetItemInstance;
	EquipmentInstance->SetSlotTag(GetInventoryTag());
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRep_ItemInstance();
	}
	return EquipmentInstance;
}

bool UInvSys_InventoryFragment_Equipment::UnEquipItemInstance()
{
	check(HasAuthority())
	if (HasEquipmentItems() == false)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("当前装备模块 %s 未装备任何物品。"), *InventoryTag.ToString())
		return false;
	}

	EquipmentInstance->RemoveFromInventory();
	EquipmentInstance = nullptr;
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRep_ItemInstance();
	}
	return true;
}

bool UInvSys_InventoryFragment_Equipment::HasEquipmentItems() const
{
	return EquipmentInstance != nullptr;
}

UInvSys_InventoryItemInstance* UInvSys_InventoryFragment_Equipment::GetEquipItemInstance() const
{
	return EquipmentInstance;
}

bool UInvSys_InventoryFragment_Equipment::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                              FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	if (EquipmentInstance && IsValid(EquipmentInstance))
	{
		bWroteSomething |= Channel->ReplicateSubobject(EquipmentInstance, *Bunch, *RepFlags);
	}
	return bWroteSomething;
}

void UInvSys_InventoryFragment_Equipment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInvSys_InventoryFragment_Equipment, EquipmentInstance, COND_None);
}

void UInvSys_InventoryFragment_Equipment::BroadcastEquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance)
{
	UWorld* MyWorld = GetWorld();
	if (MyWorld)
	{
		FInvSys_EquipItemInstanceMessage EquipItemInstanceMessage;
		EquipItemInstanceMessage.InvComp = GetInventoryComponent();
		EquipItemInstanceMessage.InventoryObjectTag = GetInventoryTag();
		EquipItemInstanceMessage.ItemInstance = NewItemInstance;

		UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(MyWorld);
		GameplayMessageSubsystem.BroadcastMessage(Inventory_Message_EquipItem, EquipItemInstanceMessage);
	}
}

void UInvSys_InventoryFragment_Equipment::BroadcastUnEquipItemInstance()
{
	UWorld* MyWorld = GetWorld();
	if (MyWorld)
	{
		FInvSys_EquipItemInstanceMessage UnEquipItemInstanceMessage;
		UnEquipItemInstanceMessage.InvComp = GetInventoryComponent();
		UnEquipItemInstanceMessage.InventoryObjectTag = GetInventoryTag();
		UnEquipItemInstanceMessage.ItemInstance = nullptr;

		UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(MyWorld);
		GameplayMessageSubsystem.BroadcastMessage(Inventory_Message_UnEquipItem, UnEquipItemInstanceMessage);
	}
}

void UInvSys_InventoryFragment_Equipment::OnRep_ItemInstance()
{
	// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Error, TEXT("[%s] OnRep Equip ItemInstance"), HasAuthority() ? TEXT("Server"):TEXT("Client"));
	if (EquipmentInstance)
	{
		BroadcastEquipItemInstance(EquipmentInstance);
	}
	else
	{
		BroadcastUnEquipItemInstance();
	}
	LastItemInstance = EquipmentInstance;
}
