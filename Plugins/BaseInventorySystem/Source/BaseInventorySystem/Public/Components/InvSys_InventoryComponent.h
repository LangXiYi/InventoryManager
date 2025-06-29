// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InvSys_InventoryComponent.generated.h"


class UInvSys_PreEditInventoryObject;
struct FInvSys_InventoryItem;
class UInvSys_InventoryContentMapping;

// todo:: 定义装备新物品的代理

/**
 * 
 * 
 * 
 * 
 */
UCLASS(Abstract, BlueprintType, meta=(BlueprintSpawnableComponent))
class BASEINVENTORYSYSTEM_API UInvSys_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInvSys_InventoryComponent();

	/**
	 * 将物品添加到装备槽
	 * @param NewItem 
	 * @param TargetSlotName 
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	virtual void AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem, FName TargetSlotName);
	
	/**
	 * 将物品添加到容器内
	 * @param NewItem 
	 * @param ContainerObj 
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	virtual void AddInventoryItemToContainer(const FInvSys_InventoryItem& NewItem, UObject* ContainerObj);

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	virtual void PostRepNotifies() override;
	
	/** Returns true if the owner's role is ROLE_Authority */
	FORCEINLINE bool HasAuthority() const;

	FORCEINLINE APlayerController* GetPlayerController() const;

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

	/** Returns the world's timer manager */
	FORCEINLINE FTimerManager& GetWorldTimerManager() const;

	FORCEINLINE bool IsLocalController() const;

	/** 仅本地控制器，由用户手动调用。 */
	UFUNCTION(BlueprintCallable)
	virtual void InitInventoryObj(APlayerController* NewPlayerController);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UInvSys_PreEditInventoryObject* GetPreEditInventoryObject(int32 Index) const;

	UFUNCTION(BlueprintNativeEvent)
	void OnInitInventoryObjects(APlayerController* NewPlayerController);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/**
	 * 构建库存列表，推荐在 Server 端调用
	 */
	void ConstructInventoryList();
	
protected:
	/** 库存对象列表，由 InventoryContentMapping 管理，支持配置背包、装备槽或其他类型 */
	UPROPERTY(Replicated, Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TArray<class UInvSys_BaseInventoryObject*> InventoryObjectList;
	
	/** 库存内容映射，使用数据资产方便管理 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TObjectPtr<UInvSys_InventoryContentMapping> InventoryContentMapping;

	/** 加速库存对象查询速度 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Component")
	TMap<FName, UInvSys_BaseInventoryObject*> InventoryObjectMap;

	/** 拥有该组件的玩家控制器。 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Component")
	APlayerController* OwningPlayer;

	/** 库存对象是否已经更新？限制初始化函数只执行一次。 */
	bool bIsInitInventoryObjects;
	
private:
	/** 初始化计时器，确保库存对象在正确的时间执行初始化函数 */
	FTimerHandle InitTimer;
};
