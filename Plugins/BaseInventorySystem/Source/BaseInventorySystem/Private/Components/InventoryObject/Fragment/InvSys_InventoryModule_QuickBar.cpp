// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/InvSys_InventoryModule_QuickBar.h"

#include "BaseInventorySystem.h"
#include "Data/InvSys_ContainerList.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"

UInvSys_InventoryModule_QuickBar::UInvSys_InventoryModule_QuickBar()
{
	QuickBarItemReferences.Init(nullptr, 10);
}

void UInvSys_InventoryModule_QuickBar::InitInventoryFragment(UObject* PreEditFragment)
{
	Super::InitInventoryFragment(PreEditFragment);

	/*
	 * 监听物品移除事件，当物品发生改变后移除绑定
	 * todo::只要在容器内就不会移除快捷栏绑定？
	 */
	if (HasAuthority())
	{
		auto ListenRemoveFunc = [this](FGameplayTag Tag, const FInvSys_InventoryItemChangedMessage& Message)
		{
			if (Message.InvComp == GetInventoryComponent())
			{
				int32 Index = QuickBarItemReferences.Find(Message.ItemInstance);
				if (QuickBarItemReferences.IsValidIndex(Index))
				{
					UpdateQuickBarItemReference(nullptr, Index);
				}
			}
		};

		UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		GameplayMessageSubsystem.RegisterListener<FInvSys_InventoryItemChangedMessage>(Inventory_Message_RemoveItem, MoveTemp(ListenRemoveFunc));
	}
}

void UInvSys_InventoryModule_QuickBar::UpdateQuickBarItemReference(UInvSys_InventoryItemInstance* ItemReference,
                                                                   int32 Index)
{
	if (ItemReference->GetInventoryComponent() != GetInventoryComponent())
	{
		return;
	}
	QuickBarItemReferences[Index] = ItemReference;
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRep_QuickBarItemReferences();
	}
}

UInvSys_InventoryItemInstance* UInvSys_InventoryModule_QuickBar::FindInventoryItemInstance(int32 Index)
{
	if (QuickBarItemReferences.IsValidIndex(Index))
	{
		return QuickBarItemReferences[Index].Get();
	}
	return nullptr;
}

void UInvSys_InventoryModule_QuickBar::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_InventoryModule_QuickBar, QuickBarItemReferences);
}

void UInvSys_InventoryModule_QuickBar::OnRep_QuickBarItemReferences()
{
	FInvSys_QuickBarChangedMessage Message;
	Message.QuickBarItems.Reserve(QuickBarItemReferences.Num());
	for (TWeakObjectPtr<UInvSys_InventoryItemInstance> ItemReference : QuickBarItemReferences)
	{
		Message.QuickBarItems.Add(ItemReference.Get());
	}
	Message.InvComp = GetInventoryComponent();

	UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	GameplayMessageSubsystem.BroadcastMessage(Inventory_Message_QuickBarChanged, Message);
}
