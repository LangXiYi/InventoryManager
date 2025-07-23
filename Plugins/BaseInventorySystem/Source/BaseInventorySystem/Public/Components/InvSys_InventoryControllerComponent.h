// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "InvSys_InventoryComponent.h"
#include "Components/ActorComponent.h"
#include "InvSys_InventoryControllerComponent.generated.h"


class UInvSys_InventoryItemInstance;
class UInvSys_InventoryComponent;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class BASEINVENTORYSYSTEM_API UInvSys_InventoryControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInvSys_InventoryControllerComponent();

public:
	/**
	 * RPC Functions
	 **/

	/**
	 * 由于放置逻辑可能存在其他不确定的属性，所以这里没有办法提前定义 RPC 函数，需要子类实现自定义的 Server RPC
	 * void Server_TryDropItemInstance(InvComp, ItemInstance, SlotTag, Args...);
	 */
	template<class T, class... Arg>
	bool TryDropItemInstance(UInvSys_InventoryComponent* InvComp, T* InItemInstance, FGameplayTag SlotTag, const Arg&... Ags)
	{
		if (bIsSuccessDragItem && InvComp)
		{
			return InvComp->TryDropItemInstance<T>(InItemInstance, SlotTag, Ags...);
		}
		return false;
	}

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TryDragItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_DropItemInstanceToWorld(UInvSys_InventoryItemInstance* InItemInstance);
	
public:
	/** Gets the game instance this component is a part of, this will return null if not called during normal gameplay */
	template <class T>
	T* GetGameInstance() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, UGameInstance>::Value, "'T' template parameter to GetGameInstance must be derived from UGameInstance");
		AActor* Owner = GetOwner();
		return Owner ? Owner->GetGameInstance<T>() : nullptr;
	}

	template <class T>
	T* GetGameInstanceChecked() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, UGameInstance>::Value, "'T' template parameter to GetGameInstance must be derived from UGameInstance");
		AActor* Owner = GetOwner();
		check(Owner);
		T* GameInstance = Owner->GetGameInstance<T>();
		check(GameInstance);
		return GameInstance;
	}

	/** Returns true if the owner's role is ROLE_Authority */
	bool HasAuthority() const;

	/** Returns the world's timer manager */
	class FTimerManager& GetWorldTimerManager() const;
	
	APlayerController* GetPlayerController();

private:
	// 标记玩家是否拖拽起了目标物品
	bool bIsSuccessDragItem = false;

	// 保存目标物品的引用
	UPROPERTY()
	TObjectPtr<UInvSys_InventoryItemInstance> DraggingItemInstance = nullptr;
};
