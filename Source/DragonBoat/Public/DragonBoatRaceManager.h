// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Datamanagement.h"
#include "DragonBoatPawn.h"
#include "DragonBoatRaceManager.generated.h"

/**
 * 龙舟比赛管理器 - 管理3条龙舟的生成、特殊效果应用、终点检测
 */
UCLASS()
class DRAGONBOAT_API ADragonBoatRaceManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ADragonBoatRaceManager();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// ========== 配置 ==========

	// 龙舟蓝图类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	TSubclassOf<class ADragonBoatPawn> DragonBoatClass;

	// 玩家龙舟生成位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	FVector PlayerSpawnLocation;

	// AI龙舟1生成位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	FVector AI1SpawnLocation;

	// AI龙舟2生成位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	FVector AI2SpawnLocation;

	// 加速Buff数值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	float SpeedUpAmount;

	// 减速Buff数值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	float SlowDownAmount;

	// Datamanagement引用（用于接收特殊效果）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	class ADatamanagement* DataManager;

	// ========== 运行时状态 ==========

	// 玩家龙舟
	UPROPERTY(BlueprintReadOnly, Category = "Race State")
	class ADragonBoatPawn* PlayerBoat;

	// AI龙舟1
	UPROPERTY(BlueprintReadOnly, Category = "Race State")
	class ADragonBoatPawn* AI1Boat;

	// AI龙舟2
	UPROPERTY(BlueprintReadOnly, Category = "Race State")
	class ADragonBoatPawn* AI2Boat;

	// 已到达终点的龙舟数量
	UPROPERTY(BlueprintReadOnly, Category = "Race State")
	int32 FinishedBoatCount;

	// 比赛是否已结束
	UPROPERTY(BlueprintReadOnly, Category = "Race State")
	bool bRaceFinished;

	// ========== 公共接口 ==========

	// 初始化比赛（生成龙舟）
	UFUNCTION(BlueprintCallable, Category = "Race")
	void InitializeRace();

	// 应用特殊效果到龙舟
	UFUNCTION(BlueprintCallable, Category = "Race")
	void ApplySpecialEffect(ESlotEffectType EffectType);

	// 龙舟到达终点
	UFUNCTION(BlueprintCallable, Category = "Race")
	void OnBoatReachedFinish(class ADragonBoatPawn* Boat);

	// 获取所有龙舟
	UFUNCTION(BlueprintPure, Category = "Race")
	TArray<ADragonBoatPawn*> GetAllBoats() const;

	// ========== UI事件 ==========

	// [事件] 比赛开始
	UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
	void OnRaceStarted();

	// [事件] 龙舟到达终点
	UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
	void OnBoatFinished(class ADragonBoatPawn* Boat, int32 Rank);

	// [事件] 比赛结束
	UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
	void OnRaceCompleted();

private:
	// 生成龙舟
	ADragonBoatPawn* SpawnBoat(const FVector& Location, EBoatType BoatType, bool bEnableCamera);

	// 绑定到Datamanagement的特殊效果事件
	void BindToDataManager();

	// 响应特殊效果触发
	UFUNCTION()
	void HandleMatchesCleared(const TArray<int32>& ClearedIndices, const TArray<FSpecialEffectData>& TriggeredEffects);
};
