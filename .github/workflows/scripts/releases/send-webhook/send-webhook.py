from discord import Webhook, AsyncWebhookAdapter
import aiohttp
import os

async def send_webhook():
    async with aiohttp.ClientSession() as session:
        webhook = Webhook.from_url(
            os.environ["DISCORD_BUILD_WEBHOOK"], adapter=AsyncWebhookAdapter(session)
        )

        # TODO - get the asset links

        embed = discord.Embed(
            title="New PCSX2 Nightly Build Available!", color=0xFF8000
        )
        embed.add_field(name="Version", value="v1.7.9999", inline=True)
        embed.add_field(name="Release Link", value="www.google.com", inline=True)
        embed.add_field(name="Installation Steps", value="www.google.com", inline=True)
        embed.add_field(
            name="Windows Downloads",
            value="- Something\n- something else",
            inline=False,
        )
        embed.add_field(name="Linux Downloads", value="- Something", inline=True)

        await webhook.send(embed=embed)

send_webhook()
