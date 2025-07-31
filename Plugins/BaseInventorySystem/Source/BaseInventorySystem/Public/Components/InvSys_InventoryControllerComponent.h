// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	 * void Server_TryDropItemInstance(InvComp, ItemInstance, SlotTag, Args...);
	 */
	template<class T, class... Arg>
	bool DropItemInstance(UInvSys_InventoryComponent* InvComp, T* InItemInstance, FGameplayTag SlotTag, const Arg&... Ags)
	{
		if (bIsSuccessDragItem && InvComp)
		{
			bIsSuccessDragItem = false;
			DraggingItemInstance = nullptr;
			return InvComp->AddItemInstance<T>(InItemInstance, SlotTag, Ags...);
		}
		return false;
	}

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TryDragItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_DropItemInstanceToWorld(UInvSys_InventoryItemInstance* InItemInstance);
	
private:
	// 标记玩家是否拖拽起了目标物品
	bool bIsSuccessDragItem = false;

	// 保存目标物品的引用
	UPROPERTY()
	TObjectPtr<UInvSys_InventoryItemInstance> DraggingItemInstance = nullptr;
};
