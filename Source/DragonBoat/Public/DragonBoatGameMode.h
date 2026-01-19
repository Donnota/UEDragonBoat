// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DragonBoatGameMode.generated.h"

// 游戏状态枚举
UENUM(BlueprintType)
enum class ERaceGameState : uint8
{
	PreRace		UMETA(DisplayName = "Pre-Race"),		// 准备阶段（倒计时）
	Racing		UMETA(DisplayName = "Racing"),			// 比赛进行中
	Paused		UMETA(DisplayName = "Paused"),			// 暂停
	Finished	UMETA(DisplayName = "Finished"),		// 比赛结束
	PostRace	UMETA(DisplayName = "Post-Race")		// 结算阶段
};

// 最终结果（蓝图可用）
USTRUCT(BlueprintType)
struct FBoatFinalResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Race Result")
	int32 BoatIndex;  // 0=玩家, 1=AI1, 2=AI2

	UPROPERTY(BlueprintReadOnly, Category = "Race Result")
	int32 FinalRank;  // 最终排名

	UPROPERTY(BlueprintReadOnly, Category = "Race Result")
	float FinishTime;  // 完成时间

	UPROPERTY(BlueprintReadOnly, Category = "Race Result")
	bool bIsPlayer;  // 是否是玩家

	FBoatFinalResult()
		: BoatIndex(0), FinalRank(1), FinishTime(0.0f), bIsPlayer(false)
	{}
};

/**
 * 龙舟竞速GameMode - 负责游戏状态管理、进度计算、排名统计
 */
UCLASS()
class DRAGONBOAT_API ADragonBoatGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADragonBoatGameMode();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========== 配置参数 ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	float RaceTrackLength;  // 赛道总长度（UE单位）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	FVector StartLinePosition;  // 起点位置

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	FVector FinishLinePosition;  // 终点位置

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	float CountdownDuration;  // 倒计时时长（秒）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	float ProgressUpdateInterval;  // 进度更新间隔（秒，如0.1或0.2）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Config")
	float RaceEndDelay;  // 第一名完成后延迟X秒结束比赛

	// ========== 龙舟引用（蓝图配置）==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Setup")
	AActor* PlayerBoat;  // 玩家龙舟

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Setup")
	AActor* AIBoat1;  // AI龙舟1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Setup")
	AActor* AIBoat2;  // AI龙舟2

	// ========== 运行时数据 ==========

	UPROPERTY(BlueprintReadOnly, Category = "Race State")
	ERaceGameState CurrentGameState;

	UPROPERTY(BlueprintReadOnly, Category = "Race State")
	float CurrentRaceTime;  // 当前比赛时间

	UPROPERTY(BlueprintReadOnly, Category = "Race State")
	int32 FinishedBoatCount;  // 已完成的龙舟数量

	// ========== 公共接口 ==========

	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void StartCountdown();

	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void StartRace();

	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void PauseRace();

	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void ResumeRace();

	// 获取龙舟当前排名
	UFUNCTION(BlueprintPure, Category = "Race Query")
	int32 GetBoatRank(int32 BoatIndex) const;

	// 获取龙舟当前进度（0.0-1.0）
	UFUNCTION(BlueprintPure, Category = "Race Query")
	float GetBoatProgress(int32 BoatIndex) const;

	// ========== 蓝图事件 ==========

	// [事件] 倒计时更新（每秒触发一次）
	UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
	void OnCountdownTick(int32 RemainingSeconds);

	// [事件] 比赛开始
	UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
	void OnRaceStarted();

	// [事件] 进度更新（定时触发，不是每帧）
	// BoatProgresses: [玩家进度, AI1进度, AI2进度]
	// BoatRanks: [玩家排名, AI1排名, AI2排名]
	UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
	void OnProgressUpdated(const TArray<float>& BoatProgresses, const TArray<int32>& BoatRanks);

	// [事件] 排名变化（只在排名改变时触发）
	UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
	void OnRankChanged(int32 BoatIndex, int32 OldRank, int32 NewRank);

	// [事件] 龙舟完成比赛
	UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
	void OnBoatFinished(int32 BoatIndex, float FinishTime, int32 FinalRank);

	// [事件] 比赛结束
	UFUNCTION(BlueprintImplementableEvent, Category = "Race Events")
	void OnRaceFinished(const TArray<FBoatFinalResult>& FinalRankings);

private:
	// 龙舟运行时数据（内部使用）
	struct FBoatRaceData
	{
		float CurrentProgress;		// 当前进度（0.0-1.0）
		int32 CurrentRank;			// 当前排名（1-3）
		float FinishTime;			// 完成时间（-1表示未完成）
		bool bHasFinished;			// 是否已完成

		FBoatRaceData()
			: CurrentProgress(0.0f)
			, CurrentRank(1)
			, FinishTime(-1.0f)
			, bHasFinished(false)
		{}
	};

	// 每条龙舟的数据
	TArray<FBoatRaceData> BoatDataArray;

	// Timer句柄
	FTimerHandle CountdownTimerHandle;
	FTimerHandle ProgressUpdateTimerHandle;
	FTimerHandle RaceEndTimerHandle;

	// 倒计时剩余秒数
	int32 CountdownRemaining;

	// ========== 内部函数 ==========

	// 倒计时Tick
	void CountdownTick();

	// 定时更新进度
	void UpdateProgress();

	// 更新排名
	void UpdateRankings();

	// 龙舟到达终点
	void OnBoatReachedFinish(int32 BoatIndex);

	// 结束比赛
	void EndRace();
};
