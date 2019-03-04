set ANSI_NULLS ON
set QUOTED_IDENTIFIER ON
go


ALTER proc [dbo].[up_quote_minute_k_line]
    @now varchar(30),
    @symbol varchar(10),
    @price numeric(10, 4),
    @amount numeric(12),
    @sum numeric(16, 4)
as
    declare @t5 datetime, @t15 datetime, @t30 datetime, @t60 datetime,
        @var char(1), @mkt char(1), @s5 int, @s15 int, @s30 int, @s60 int,
        @price_max numeric(10, 4), @price_min numeric(10, 4),
        @old_max numeric(10, 4), @old_min numeric(10, 4), @old_amount numeric(12), @old_sum numeric(16, 4)

    -- 数据转换
    if @symbol = '999800'
        select @symbol = 'HSI', @var = 'Z', @mkt = 'H'
    else if @symbol = '999801'
        select @symbol = 'HSNF', @var = 'Z', @mkt = 'H'
    else if @symbol = '999802'
        select @symbol = 'HSNU', @var = 'Z', @mkt = 'H'
    else if @symbol = '999803'
        select @symbol = 'HSNP', @var = 'Z', @mkt = 'H'
    else if @symbol = '999804'
        select @symbol = 'HSNC', @var = 'Z', @mkt = 'H'
    else if @symbol = '999806'
        select @symbol = 'HSF50', @var = 'Z', @mkt = 'H'
    else if @symbol = '999807'
        select @symbol = 'HSCC', @var = 'Z', @mkt = 'H'
    else if @symbol = '999811'
        select @symbol = 'HSHLCI', @var = 'Z', @mkt = 'H'
    else if @symbol = '999812'
        select @symbol = 'HSHSCI', @var = 'Z', @mkt = 'H'
    else if @symbol = '999813'
        select @symbol = 'HSHMCI', @var = 'Z', @mkt = 'H'
    else if @symbol = '999814'
        select @symbol = 'HSHCI', @var = 'Z', @mkt = 'H'
    else if @symbol = '2823'
        select @symbol = '02823', @var = 'U', @mkt = 'H'
    else if @symbol = '399300'
        select @symbol = '399300', @var = 'Z', @mkt = 'Z'
    else
        return

    -- 获得该symbol的相应时间的最新一条记录的序号
    select @s5 = max(seq) from QOT_D_FACT_FIVE where SEC_CD = @symbol and PUB_DT <= @now
    select @s15 = max(seq) from QOT_D_FACT_FIFT where SEC_CD = @symbol and PUB_DT <= @now
    select @s30 = max(seq) from QOT_D_FACT_THIR where SEC_CD = @symbol and PUB_DT <= @now
    select @s60 = max(seq) from QOT_D_FACT_SIXT where SEC_CD = @symbol and PUB_DT <= @now

    -- 获得最后一条记录更新时间
    select @t5 = PUB_DT from QOT_D_FACT_FIVE where SEC_CD = @symbol and seq = @s5
    select @t15 = PUB_DT from QOT_D_FACT_FIFT where SEC_CD = @symbol and seq = @s15
    select @t30 = PUB_DT from QOT_D_FACT_THIR where SEC_CD = @symbol and seq = @s30
    select @t60 = PUB_DT from QOT_D_FACT_SIXT where SEC_CD = @symbol and seq = @s60

    if @t5 is null or abs(datediff(minute, @now, @t5)) >= 5
        insert QOT_D_FACT_FIVE(PUB_DT, UPDT_TM, SEC_CD, VAR_CL, MKT_CL, f0020, f0030, f0040, f0050, f0060, f0070)
            values(@now, getdate(), @symbol, @var, @mkt, @price, @price, @price, @price, @amount, @sum)
    else
        begin
        select @old_max = f0030, @old_min = f0040 from QOT_D_FACT_FIVE where seq = @s5
        select @price_max = case when @old_max > @price then @old_max else @price end
        select @price_min = case when @old_min < @price then @old_min else @price end
        update QOT_D_FACT_FIVE set UPDT_TM = getdate(),
            f0030 = @price_max, f0040 = @price_min, f0050 = @price, f0060 = f0060 + @amount, f0070 = f0070 + @sum
            where seq = @s5
        end

    if @t15 is null or abs(datediff(minute, @now, @t15)) >= 15
        insert QOT_D_FACT_FIFT(PUB_DT, UPDT_TM, SEC_CD, VAR_CL, MKT_CL, f0020, f0030, f0040, f0050, f0060, f0070)
            values(@now, getdate(), @symbol, @var, @mkt, @price, @price, @price, @price, @amount, @sum)
    else
        begin
        select @old_max = f0030, @old_min = f0040 from QOT_D_FACT_FIFT where seq = @s15
        select @price_max = case when @old_max > @price then @old_max else @price end
        select @price_min = case when @old_min < @price then @old_min else @price end
        update QOT_D_FACT_FIFT set UPDT_TM = getdate(),
            f0030 = @price_max, f0040 = @price_min, f0050 = @price, f0060 = f0060 + @amount, f0070 = f0070 + @sum
            where seq = @s15
        end

    if @t30 is null or abs(datediff(minute, @now, @t30)) >= 30
        insert QOT_D_FACT_THIR(PUB_DT, UPDT_TM, SEC_CD, VAR_CL, MKT_CL, f0020, f0030, f0040, f0050, f0060, f0070)
            values(@now, getdate(), @symbol, @var, @mkt, @price, @price, @price, @price, @amount, @sum)
    else
        begin
        select @old_max = f0030, @old_min = f0040 from QOT_D_FACT_THIR where seq = @s30
        select @price_max = case when @old_max > @price then @old_max else @price end
        select @price_min = case when @old_min < @price then @old_min else @price end
        update QOT_D_FACT_THIR set UPDT_TM = getdate(),
            f0030 = @price_max, f0040 = @price_min, f0050 = @price, f0060 = f0060 + @amount, f0070 = f0070 + @sum
            where seq = @s30
        end

    if @t60 is null or abs(datediff(minute, @now, @t60)) >= 60
        insert QOT_D_FACT_SIXT(PUB_DT, UPDT_TM, SEC_CD, VAR_CL, MKT_CL, f0020, f0030, f0040, f0050, f0060, f0070)
            values(@now, getdate(), @symbol, @var, @mkt, @price, @price, @price, @price, @amount, @sum)
    else
        begin
        select @old_max = f0030, @old_min = f0040 from QOT_D_FACT_SIXT where seq = @s60
        select @price_max = case when @old_max > @price then @old_max else @price end
        select @price_min = case when @old_min < @price then @old_min else @price end
        update QOT_D_FACT_SIXT set UPDT_TM = getdate(),
            f0030 = @price_max, f0040 = @price_min, f0050 = @price, f0060 = f0060 + @amount, f0070 = f0070 + @sum
            where seq = @s60
        end

