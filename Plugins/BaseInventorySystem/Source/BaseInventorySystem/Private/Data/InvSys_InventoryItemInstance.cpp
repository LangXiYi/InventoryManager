// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/InvSys_InventoryItemInstance.h"

#include "BaseInventorySystem.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"

UInvSys_InventoryItemInstance::UInvSys_InventoryItemInstance(const FObjectInitializer& ObjectInitializer)
{
	UObject* MyOuter = GetOuter();
	if (MyOuter)
	{
		if (MyOuter->IsA<UInvSys_InventoryComponent>())
		{
			InvComp = Cast<UInvSys_InventoryComponent>(MyOuter);
			Owner = InvComp->GetOwner();
		}
		else
		{
			UE_LOG(LogInventorySystem, Error, TEXT("库存物品实例的 Outer 必须是 InventoryComponent 及其子类！！！"))
		}
	}
}

bool UInvSys_InventoryItemInstance::HasAuthority() const
{
	check(Owner)
	return Owner->HasAuthority();
}

ENetMode UInvSys_InventoryItemInstance::GetNetMode() const
{
	check(Owner)
	return Owner->GetNetMode();
}

void UInvSys_InventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_InventoryItemInstance, ItemDefinition);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, ItemUniqueID);
	DOREPLIFETIME(UInvSys_InventoryItemInstance, SlotTag);
}

void UInvSys_InventoryItemInstance::OnRep_SlotTag(const FGameplayTag& OldSlotTag)
{
	ON_REP_PROPERTY(SlotTag);
}

void UInvSys_InventoryItemInstance::Execute_SlotTag(const FGameplayTag& OldSlotTag)
{
	LastSlotTag = OldSlotTag;
}

void UInvSys_InventoryItemInstance::SetItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> NewItemDef)
{
	ItemDefinition = NewItemDef;
}

void UInvSys_InventoryItemInstance::SetItemUniqueID(FGuid Guid)
{
	ItemUniqueID = Guid;
}

void UInvSys_InventoryItemInstance::SetSlotTag(FGameplayTag Tag)
{
	LastSlotTag = SlotTag;
	SlotTag = Tag;
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRep_SlotTag(LastSlotTag);
	}
}

void UInvSys_InventoryItemInstance::SetInventoryComponent(UInvSys_InventoryComponent* NewInvComp)
{
	InvComp = NewInvComp;
}

void UInvSys_InventoryItemInstance::ReplicatedProperties()
{
	if (bIsReadyReplicatedProperties == false)
	{
		return;
	}
	// 遍历所有需要处理的属性并执行对应的函数！！
	for (FInvSys_ItemInstancePropertyHandle OnRepProperty : RegisterPropertyArrays)
	{
		OnRepProperty.OnRepCallback();
	}
	bIsReadyReplicatedProperties = false;
	RegisterPropertyArrays.Reset();
}

void UInvSys_InventoryItemInstance::RemoveFromInventory()
{
	// SlotTag = FGameplayTag(); //不要移除这些属性
	// InvComp = nullptr;
	Entry_Private = nullptr;
}

const UInvSys_InventoryItemFragment* UInvSys_InventoryItemInstance::FindFragmentByClass(TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const
{
	if ((ItemDefinition != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UInvSys_InventoryItemDefinition>(ItemDefinition)->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}
