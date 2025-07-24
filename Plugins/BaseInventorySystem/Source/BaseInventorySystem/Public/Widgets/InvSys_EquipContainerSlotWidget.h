// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "InvSys_EquipSlotWidget.h"
#include "Components/NamedSlot.h"
#include "InvSys_EquipContainerSlotWidget.generated.h"

class UInvSys_BaseEquipContainerObject;
class UInvSys_ContainerLayoutWidget;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_EquipContainerSlotWidget : public UInvSys_EquipSlotWidget
{
	GENERATED_BODY()

	friend UInvSys_BaseEquipContainerObject;

public:
	// void AddContainerLayout(TObjectPtr<UInvSys_InventoryWidget> ContainerLayout);

	UFUNCTION(BlueprintImplementableEvent)
	UInvSys_InventoryWidget* GetContainerLayoutWidget(); // DEPRECATED

	template<class T>
	T* GetContainerLayoutWidget() const
	{
		if (NS_ContainerGridLayout->HasAnyChildren())
		{
			UWidget* ChildAt = NS_ContainerGridLayout->GetChildAt(0);
			if (ChildAt->IsA<T>())
			{
				return (T*)NS_ContainerGridLayout->GetChildAt(0);
			}
		}
		else
		{
			UE_LOG(LogInventorySystem, Warning, TEXT("未在 NS_ContainerGridLayout 下发现任何控件，容器布局必须添加在 NS_ContainerGridLayout 下。"))
		}
		return nullptr;
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UNamedSlot> NS_ContainerGridLayout;
};