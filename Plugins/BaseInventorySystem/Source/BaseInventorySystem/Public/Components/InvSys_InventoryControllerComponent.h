// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "InvSys_InventoryComponent.h"
#include "Components/ControllerComponent.h"
#include "InvSys_InventoryControllerComponent.generated.h"

class UInvSys_InventoryItemInstance;
class UInvSys_InventoryComponent;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class BASEINVENTORYSYSTEM_API UInvSys_InventoryControllerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UInvSys_InventoryControllerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * RPC Functions
	 **/

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_EquipItemDefinition(UInvSys_InventoryComponent* InvComp,TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, FGameplayTag SlotTag);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_EquipItemInstance(UInvSys_InventoryComponent* InvComp,UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_RestoreItemInstance(UInvSys_InventoryComponent* InvComp,UInvSys_InventoryItemInstance* InItemInstance);

	/**
	 * 由于放置逻辑可能存在其他不确定的属性，所以这里没有办法提前定义 RPC 函数，需要子类实现自定义的 Server RPC
	 * void Server_TryDropItemInstance(InvComp, ItemInstance, SlotTag...);
	 */
	template<class T, class... Arg>
	bool DropItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag, const Arg&... Args)
	{
		if (InvComp == nullptr)
		{
			UE_LOG(LogInventorySystem, Error, TEXT("%hs Failed, Inventory Component is nullptr."), __FUNCTION__)
			return false;
		}
		if (DraggingItemInstance == nullptr)
		{
			UE_LOG(LogInventorySystem, Error, TEXT("%hs Failed, Dragging ItemInstance is nullptr."), __FUNCTION__)
			return false;
		}
		if (DraggingItemInstance != InItemInstance)
		{
			UE_LOG(LogInventorySystem, Error, TEXT("%hs Failed, DraggingItemInstance != InItemInstance."), __FUNCTION__)
			return false;
		}

		DraggingItemInstance = nullptr;
		return InvComp->DropItemInstance<T>(InItemInstance, SlotTag, Args...);
	}

	/**
	 * 由于放置逻辑可能存在其他不确定的属性，所以这里没有办法提前定义 RPC 函数，需要子类实现自定义的 Server RPC
	 * void Server_TryDropItemInstance(InvComp, ItemInstance, SlotTag...);
	 */
	template<class T, class... Arg>
	T* DropAndAddItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag, const Arg&... Args)
	{
		if (DraggingItemInstance.IsValid() && DraggingItemInstance == InItemInstance && InvComp)
		{
			DraggingItemInstance = nullptr;
			return InvComp->AddItemInstance<T>(InItemInstance, SlotTag, Args...);
		}
		return nullptr;
	}

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_DragItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_CancelDragItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_DragAndRemoveItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_DropItemInstanceToWorld(UInvSys_InventoryItemInstance* InItemInstance);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_DraggingItemInstance)
	TWeakObjectPtr<UInvSys_InventoryItemInstance> DraggingItemInstance = nullptr;
	UFUNCTION()
	void OnRep_DraggingItemInstance();
};
