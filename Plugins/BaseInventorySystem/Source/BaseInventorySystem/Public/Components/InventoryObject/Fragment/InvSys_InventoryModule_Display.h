// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_CommonType.h"
#include "InvSys_InventoryModule.h"
#include "Widgets/InvSys_InventoryWidget.h"
#include "InvSys_InventoryModule_Display.generated.h"

class UInvSys_InventoryWidget;

UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryModule_Display : public UInvSys_InventoryModule
{
	GENERATED_BODY()

public:
	UInvSys_InventoryModule_Display();
	
	virtual void InitInventoryFragment(UObject* PreEditFragment) override;

	virtual void RefreshInventoryFragment() override;

	// 创建显示控件，创建成功后会自动将该控件添加至HUD管理，若控件已创建则之间返回该控件。
	UFUNCTION(BlueprintCallable, Category = "Inventory Module|Display")
	virtual UInvSys_InventoryWidget* TryCreateDisplayWidget(APlayerController* PC);

	TSubclassOf<UUserWidget> GetDisplayWidgetClass() const
	{
		return DisplayWidgetClass;
	}

	UFUNCTION(BlueprintPure, Category = "Inventory Module|Display")
	UInvSys_InventoryWidget* GetDisplayWidget() const;

	template<class T>
	T* GetDisplayWidget() const
	{
		return (T*)GetDisplayWidget();
	}

	UFUNCTION(BlueprintPure, Category = "Inventory Module|Display")
	FORCEINLINE EInvSys_InventoryWidgetActivity GetInventoryWidgetActivity() const
	{
		return WidgetActivity;
	}

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Module|Display")
	TSubclassOf<UInvSys_InventoryWidget> DisplayWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Module|Display")
	EInvSys_InventoryWidgetActivity WidgetActivity = EInvSys_InventoryWidgetActivity::Permanent;

	UPROPERTY()
	TWeakObjectPtr<UInvSys_InventoryWidget> DisplayWidget = nullptr;
};
