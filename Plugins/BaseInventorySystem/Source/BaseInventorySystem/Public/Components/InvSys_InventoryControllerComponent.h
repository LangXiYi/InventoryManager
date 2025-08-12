// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "InvSys_InventoryComponent.h"
#include "InvSys_InventoryControllerComponent.generated.h"

class UInvSys_InventoryHUD;
class UInvSys_InventoryItemInstance;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class BASEINVENTORYSYSTEM_API UInvSys_InventoryControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInvSys_InventoryControllerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// todo::如果自定义 HUD，请重载该函数并添加自己的处理逻辑
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Player Inventory Component")
	virtual UInvSys_InventoryHUD* ConstructInventoryHUD();

	UInvSys_InventoryHUD* GetInventoryHUD() const;

	/**
	 * 由于放置逻辑可能存在其他不确定的属性，所以这里没有办法提前定义 RPC 函数，需要子类实现自定义的 Server RPC
	 * void Server_TryDropItemInstance(InvComp, ItemInstance, SlotTag...);
	 */
	template<class T, class... Arg>
	bool DropItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag, const Arg&... Args);

	/**
	 * 由于放置逻辑可能存在其他不确定的属性，所以这里没有办法提前定义 RPC 函数，需要子类实现自定义的 Server RPC
	 * void Server_TryDropItemInstance(InvComp, ItemInstance, SlotTag...);
	 */
	template<class T, class... Arg>
	T* DropAndAddItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag, const Arg&... Args);

public:
	/**
	 * RPC Functions
	 **/

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_EquipItemDefinition(UInvSys_InventoryComponent* InvComp,TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, FGameplayTag SlotTag);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_EquipItemInstance(UInvSys_InventoryComponent* InvComp,UInvSys_InventoryItemInstance* ItemInstance, FGameplayTag InventoryTag);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_DragItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_CancelDragItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_DragAndRemoveItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_DropItemInstanceToWorld(UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Player Inventory Component")
	void Multi_DropItemInstanceToWorld(UInvSys_InventoryItemInstance* InItemInstance, const FTransform& Transform);

protected:
	FORCEINLINE bool HasAuthority() const;

	void SetDraggingItemInstance(UInvSys_InventoryItemInstance* NewDragItemInstance);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DraggingItemInstance, Category = "Player Inventory Component")
	TWeakObjectPtr<UInvSys_InventoryItemInstance> DraggingItemInstance = nullptr;
	UFUNCTION()
	void OnRep_DraggingItemInstance(const TWeakObjectPtr<UInvSys_InventoryItemInstance>& OldDraggingItemInstance);

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Player Inventory Component")
	TSubclassOf<UInvSys_InventoryHUD> InventoryHUDClass;

	UPROPERTY(BlueprintReadOnly, Category = "Player Inventory Component")
	UInvSys_InventoryHUD* InventoryHUD = nullptr;
};

template <class T, class ... Arg>
bool UInvSys_InventoryControllerComponent::DropItemInstance(UInvSys_InventoryComponent* InvComp,
	UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag, const Arg&... Args)
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

	SetDraggingItemInstance(nullptr);
	return InvComp->DropItemInstance<T>(InItemInstance, SlotTag, Args...);
}

template <class T, class ... Arg>
T* UInvSys_InventoryControllerComponent::DropAndAddItemInstance(UInvSys_InventoryComponent* InvComp,
	UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag, const Arg&... Args)
{
	if (DraggingItemInstance.IsValid() && DraggingItemInstance == InItemInstance && InvComp)
	{
		SetDraggingItemInstance(nullptr);
		return InvComp->AddItemInstance<T>(InItemInstance, SlotTag, Args...);
	}
	return nullptr;
}
